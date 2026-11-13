// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/extractor.h"
#include "pzip/utils.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace pzip {

namespace {

class ScopedFd {
public:
    ScopedFd() = default;
    explicit ScopedFd(int descriptor) : descriptor_(descriptor) {}

    ~ScopedFd() { reset(); }

    ScopedFd(const ScopedFd&) = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    ScopedFd(ScopedFd&& other) noexcept : descriptor_(other.release()) {}

    ScopedFd& operator=(ScopedFd&& other) noexcept {
        if (this != &other)
            reset(other.release());
        return *this;
    }

    int get() const { return descriptor_; }

    int release() {
        int descriptor = descriptor_;
        descriptor_ = -1;
        return descriptor;
    }

    void reset(int descriptor = -1) {
        if (descriptor_ >= 0)
            ::close(descriptor_);
        descriptor_ = descriptor;
    }

private:
    int descriptor_ = -1;
};

std::string descriptorError(const std::string& action, const fs::path& path, int errorNumber) {
    return action + " " + path.string() + ": " + std::strerror(errorNumber);
}

int openDirectoryBeneath(int baseDescriptor, const fs::path& relativePath, bool create,
                         std::string& errorMessage) {
    ScopedFd current(::openat(baseDescriptor, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (current.get() < 0) {
        errorMessage = descriptorError("Cannot open extraction directory", relativePath, errno);
        return -1;
    }

    for (const auto& component : relativePath) {
        const std::string name = component.string();
        if (name.empty() || name == ".")
            continue;
        if (name == "..") {
            errorMessage = "Unsafe path component in archive: " + relativePath.string();
            return -1;
        }

        ScopedFd next(::openat(current.get(), name.c_str(),
                               O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
        if (next.get() < 0 && errno == ENOENT && create) {
            if (::mkdirat(current.get(), name.c_str(), 0777) < 0 && errno != EEXIST) {
                errorMessage = descriptorError("Cannot create directory", relativePath, errno);
                return -1;
            }
            next.reset(::openat(current.get(), name.c_str(),
                                O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
        }

        if (next.get() < 0) {
            errorMessage = descriptorError("Cannot open parent directory", relativePath, errno);
            return -1;
        }
        current = std::move(next);
    }

    return current.release();
}

bool writeAll(int descriptor, const std::vector<uint8_t>& data, int& errorNumber) {
    size_t written = 0;
    while (written < data.size()) {
        const ssize_t result = ::write(descriptor, data.data() + written, data.size() - written);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            errorNumber = errno;
            return false;
        }
        if (result == 0) {
            errorNumber = EIO;
            return false;
        }
        written += static_cast<size_t>(result);
    }
    return true;
}

void setModificationTime(int descriptor, const ZipEntry& entry) {
    const time_t modificationTime = ZipReader::dosToTime(entry.header.modDate, entry.header.modTime);
    const timespec times[2] = {
        {modificationTime, 0},
        {modificationTime, 0}
    };
    ::futimens(descriptor, times);
}

} // namespace

// ============================================================================
// Extractor 实现
// ============================================================================

Extractor::Extractor(const fs::path& outputDir, const ExtractorOptions& options)
    : outputDir_(outputDir)
    , options_(options)
{
    std::error_code ec;
    absoluteOutputDir_ = fs::absolute(outputDir, ec);
    if (ec) {
        absoluteOutputDir_ = outputDir;
    }

    // 确定并发数
    size_t concurrency = options_.concurrency;
    if (concurrency == 0) {
        concurrency = std::thread::hardware_concurrency();
    }
    
    // 创建解压线程池
    auto executor = [this](ExtractTask* task) -> Error {
        return extractFile(task);
    };
    fileWorkerPool_ = std::make_unique<WorkerPool<ExtractTask>>(
        executor, concurrency, concurrency * 2
    );
}

Extractor::~Extractor() {
    close();
}

Error Extractor::extract(const fs::path& archivePath) {
    // 创建读取器
    reader_ = std::make_unique<ZipReader>(archivePath);
    
    // 打开 ZIP 文件
    Error err = reader_->open();
    if (err) return err;
    
    // 确保输出目录存在
    std::error_code ec;
    fs::create_directories(absoluteOutputDir_, ec);
    if (ec) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Cannot create output directory");
    }

    canonicalOutputDir_ = fs::weakly_canonical(absoluteOutputDir_, ec);
    if (ec) {
        return Error(ErrorCode::FILE_WRITE_ERROR,
                     "Cannot resolve output directory: " + absoluteOutputDir_.string());
    }

    std::string rootError;
    ScopedFd filesystemRoot(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (filesystemRoot.get() < 0) {
        return Error(ErrorCode::FILE_WRITE_ERROR,
                     descriptorError("Cannot open filesystem root", "/", errno));
    }
    outputRootFd_ = openDirectoryBeneath(filesystemRoot.get(),
                                         canonicalOutputDir_.relative_path(),
                                         false, rootError);
    if (outputRootFd_ < 0) {
        return Error(ErrorCode::FILE_WRITE_ERROR, rootError);
    }
    
    // 获取所有条目
    const auto& entries = reader_->entries();
    totalFiles_ = entries.size();
    
    // 创建任务
    std::vector<std::unique_ptr<ExtractTask>> tasks;
    tasks.reserve(entries.size());
    
    for (const auto& entry : entries) {
        const fs::path output = outputPath(entry.header.name);
        if (output.empty()) {
            ::close(outputRootFd_);
            outputRootFd_ = -1;
            return Error(ErrorCode::INVALID_ARCHIVE, "Unsafe path in archive: " + entry.header.name);
        }

        auto task = std::make_unique<ExtractTask>();
        task->entry = &entry;
        task->relativePath = output;
        tasks.push_back(std::move(task));
    }
    
    // 启动线程池
    fileWorkerPool_->start();
    
    // 入队任务
    for (auto& task : tasks) {
        if (cancelled_) break;
        fileWorkerPool_->enqueue(task.release());
    }
    
    // 等待完成
    err = fileWorkerPool_->close();

    ::close(outputRootFd_);
    outputRootFd_ = -1;
    
    return err;
}

Error Extractor::extractFile(ExtractTask* task) {
    if (cancelled_) {
        delete task;
        return Error(ErrorCode::CANCELLED, "Operation cancelled");
    }
    
    const auto& entry = *task->entry;
    const auto& relativePath = task->relativePath;
    
    Error err;

    if (entry.isDirectory()) {
        err = writeDirectory(relativePath, entry);
    } else {
        err = writeFile(relativePath, entry);
    }
    
    if (!err) {
        task->success = true;
    }
    
    // 更新进度
    processedFiles_++;
    if (options_.progress) {
        options_.progress(processedFiles_, totalFiles_);
    }
    
    delete task;
    return err;
}

Error Extractor::writeDirectory(const fs::path& relativePath, const ZipEntry& entry) {
    if (relativePath == ".") {
        return Error();
    }

    std::string openError;
    ScopedFd parent(openDirectoryBeneath(outputRootFd_, relativePath.parent_path(),
                                         true, openError));
    if (parent.get() < 0) {
        return Error(ErrorCode::FILE_WRITE_ERROR, openError);
    }

    const std::string name = relativePath.filename().string();
    if (name.empty() || name == "." || name == "..") {
        return Error(ErrorCode::INVALID_ARCHIVE, "Invalid directory path: " + relativePath.string());
    }

    if (::mkdirat(parent.get(), name.c_str(), 0777) < 0 && errno != EEXIST) {
        return Error(ErrorCode::FILE_WRITE_ERROR,
                     descriptorError("Cannot create directory", relativePath, errno));
    }

    ScopedFd directory(::openat(parent.get(), name.c_str(),
                                O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    if (directory.get() < 0) {
        if (!options_.overwrite)
            return Error();

        if (::unlinkat(parent.get(), name.c_str(), 0) < 0) {
            return Error(ErrorCode::FILE_WRITE_ERROR,
                         descriptorError("Cannot replace directory entry", relativePath, errno));
        }
        if (::mkdirat(parent.get(), name.c_str(), 0777) < 0) {
            return Error(ErrorCode::FILE_WRITE_ERROR,
                         descriptorError("Cannot create directory", relativePath, errno));
        }
        directory.reset(::openat(parent.get(), name.c_str(),
                                 O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
        if (directory.get() < 0) {
            return Error(ErrorCode::FILE_WRITE_ERROR,
                         descriptorError("Cannot open created directory", relativePath, errno));
        }
    }

    setPermissions(directory.get(), entry);
    return Error();
}

Error Extractor::writeFile(const fs::path& relativePath, const ZipEntry& entry) {
    std::vector<uint8_t> data;
    Error err = reader_->readDecompressed(entry, data);
    if (err)
        return err;

    std::string openError;
    ScopedFd parent(openDirectoryBeneath(outputRootFd_, relativePath.parent_path(),
                                         true, openError));
    if (parent.get() < 0) {
        return Error(ErrorCode::FILE_WRITE_ERROR, openError);
    }

    const std::string name = relativePath.filename().string();
    if (name.empty() || name == "." || name == "..") {
        return Error(ErrorCode::INVALID_ARCHIVE, "Invalid file path: " + relativePath.string());
    }

    int flags = O_WRONLY | O_CREAT | O_CLOEXEC | O_NOFOLLOW;
    flags |= options_.overwrite ? O_TRUNC : O_EXCL;

    ScopedFd output(::openat(parent.get(), name.c_str(), flags, 0666));
    if (output.get() < 0) {
        if (!options_.overwrite && errno == EEXIST)
            return Error();
        return Error(ErrorCode::FILE_WRITE_ERROR,
                     descriptorError("Cannot create file", relativePath, errno));
    }

    int writeError = 0;
    if (!writeAll(output.get(), data, writeError)) {
        return Error(ErrorCode::FILE_WRITE_ERROR,
                     descriptorError("Cannot write file", relativePath, writeError));
    }

    setPermissions(output.get(), entry);
    setModificationTime(output.get(), entry);
    return Error();
}

Error Extractor::setPermissions(int fileDescriptor, const ZipEntry& entry) {
    if (!options_.preservePermissions) {
        return Error();
    }
    
    const mode_t mode = utils::zipAttrToMode(entry.header.externalAttr);
    if (mode != 0) {
        ::fchmod(fileDescriptor, mode);
    }
    
    return Error();
}

fs::path Extractor::outputPath(const std::string& name) const {
    if (name.find('\0') != std::string::npos)
        return {};

    const fs::path localPath = utils::fromZipPath(name);
    if (localPath.empty())
        return {};

    // 拒绝绝对路径和包含 ".." 的路径分量，防止跳出解压目录。
    if (localPath.is_absolute() || localPath.has_root_name() || localPath.has_root_directory())
        return {};
    for (const auto& part : localPath) {
        if (part == "..")
            return {};
    }

    fs::path normalizedPath = localPath.lexically_normal();
    if (!normalizedPath.empty() && normalizedPath.filename().empty())
        normalizedPath = normalizedPath.parent_path();
    if (normalizedPath.empty() || normalizedPath.is_absolute()
        || normalizedPath.has_root_name() || normalizedPath.has_root_directory()) {
        return {};
    }
    for (const auto& part : normalizedPath) {
        if (part == "..")
            return {};
    }
    return normalizedPath;
}

void Extractor::cancel() {
    cancelled_ = true;
    if (fileWorkerPool_) {
        fileWorkerPool_->cancel();
    }
}

Error Extractor::close() {
    if (outputRootFd_ >= 0) {
        ::close(outputRootFd_);
        outputRootFd_ = -1;
    }
    if (reader_) {
        reader_->close();
    }
    return Error();
}

void Extractor::setProgressCallback(ProgressCallback callback) {
    options_.progress = std::move(callback);
}

} // namespace pzip

