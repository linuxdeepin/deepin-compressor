// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extraction_output.h"

#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace dcarchive {

ExtractionOutput::~ExtractionOutput()
{
    if (m_file >= 0)
        ::close(m_file);
    if (!m_temporary.isEmpty())
        ::unlinkat(m_parent, m_temporary.constData(), 0);
    if (m_parent >= 0)
        ::close(m_parent);
}

bool ExtractionOutput::fail(int error)
{
    m_error = (error == ENOSPC || error == EDQUOT)
            ? ArchiveError::DiskFull : ArchiveError::PermissionDenied;
    return false;
}

bool ExtractionOutput::open(const QString &path)
{
    Q_ASSERT(m_parent < 0);
    const QFileInfo destination(path);
    m_name = QFile::encodeName(destination.fileName());
    const QByteArray directory = QFile::encodeName(destination.path());
    if (m_name.isEmpty() || m_name == "." || m_name == ".."
            || m_name.contains('\0') || directory.contains('\0'))
        return fail(EINVAL);

    // Pin the parent for creation, cleanup and publication of the temporary file.
    m_parent = ::open(directory.constData(), O_PATH | O_DIRECTORY | O_CLOEXEC);
    if (m_parent < 0)
        return fail(errno);

    struct stat info;
    if (::fstatat(m_parent, m_name.constData(), &info, AT_SYMLINK_NOFOLLOW) == 0) {
        if (!S_ISREG(info.st_mode)
                || ::faccessat(m_parent, m_name.constData(), W_OK, AT_EACCESS) != 0)
            return fail(EACCES);
        m_mode = info.st_mode & 0777;
        m_preserveMode = true;
    } else if (errno != ENOENT) {
        return fail(errno);
    }

    // A fixed-length temporary name leaves room for a NAME_MAX destination.
    for (int attempt = 0; attempt < 8; ++attempt) {
        const QByteArray name = ".dc-extract-" + QUuid::createUuid().toRfc4122().toHex();
        if (name == m_name)
            continue;
        // New files keep the ordinary umask/default ACL; replacements keep the old mode.
        m_file = ::openat(m_parent, name.constData(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC,
                          m_preserveMode ? 0600 : 0666);
        if (m_file >= 0) {
            m_temporary = name;
            return true;
        }
        if (errno != EEXIST)
            return fail(errno);
    }
    return fail(EEXIST);
}

bool ExtractionOutput::write(const char *data, qint64 size)
{
    if (m_file < 0 || m_error != ArchiveError::None)
        return false;
    while (size > 0) {
        const ssize_t count = ::write(m_file, data, size_t(size));
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            return fail(count < 0 ? errno : EIO);
        data += count;
        size -= count;
    }
    return true;
}

bool ExtractionOutput::commit()
{
    if (m_file < 0 || m_error != ArchiveError::None)
        return false;
    if (m_preserveMode && ::fchmod(m_file, m_mode) != 0)
        return fail(errno);

    const int file = m_file;
    m_file = -1;
    if (::close(file) != 0)
        return fail(errno);

    if (::renameat(m_parent, m_temporary.constData(), m_parent, m_name.constData()) != 0)
        return fail(errno);
    m_temporary.clear();
    return true;
}

bool ExtractionOutput::keepIncompleteFile()
{
    if (m_file < 0 || m_error != ArchiveError::None || m_preserveMode)
        return false;
    const int file = m_file;
    m_file = -1;
    if (::close(file) != 0)
        return fail(errno);
    // Cancellation must not replace a destination that appeared during extraction.
    if (::renameat2(m_parent, m_temporary.constData(), m_parent, m_name.constData(), RENAME_NOREPLACE) != 0)
        return fail(errno);
    m_temporary.clear();
    return true;
}

} // namespace dcarchive
