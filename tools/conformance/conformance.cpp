// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 归档后端行为观测器，输出可版本化、可比对的 JSON。
//
// 用法：
//   conformance --backend libzip --so <libzipplugin.so> --op list --sample <a.zip>
//   conformance ... --op extract --target <dir>
//   conformance ... --op add --input <f1> [--input <f2>] --archive <out.zip> --emit <zip|tar|tar.gz|7z>
//   conformance ... --respond overwrite|skip|cancel     （Query 自动应答策略）
//   输出 JSON 到 stdout 或 --out <file>

#include "archiveinterface.h"
#include "kpluginloader.h"
#include "kpluginfactory.h"
#include "kpluginmetadata.h"
#include "datamanager.h"
#include "mimetypes.h"   // 复用生产环境的 MIME 判定（determineMimeType）
#include "queries.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QProcess>
#include <QTemporaryDir>
#include <QTimer>
#include <QDebug>

#include <algorithm>
#include <cstdio>

Q_DECLARE_METATYPE(KPluginMetaData)

namespace {

struct Observation {
    int finish = 0;
    int errorType = 0;
    int finishedSignals = 0;
    int cancelSignals = 0;
    bool async = false;
    QVector<double> progress;
    QStringList curFiles;
    QJsonArray queries;
    bool waitTimedOut = false;
};

QString sha256OfFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QStringLiteral("<unreadable>");
    QCryptographicHash h(QCryptographicHash::Sha256);
    if (!h.addData(&f))
        return QStringLiteral("<hash-failed>");
    return QString::fromLatin1(h.result().toHex());
}

// 文件树快照：路径/类型/大小/权限/修改时间/软链目标/内容哈希
QJsonArray snapshotTree(const QString &root)
{
    QJsonArray arr;
    if (root.isEmpty() || !QFileInfo::exists(root))
        return arr;

    QFileInfo rootInfo(root);
    QDirIterator it(root, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        const QString rel = rootInfo.absoluteFilePath().isEmpty()
                ? fi.fileName()
                : QDir(rootInfo.absoluteFilePath()).relativeFilePath(fi.absoluteFilePath());

        QJsonObject o;
        o.insert(QStringLiteral("path"), rel);
        if (fi.isSymLink()) {
            o.insert(QStringLiteral("type"), QStringLiteral("link"));
            // 软链目标若落在解压根内，转成相对路径，避免临时目录名污染比对
            QString linkTarget = fi.symLinkTarget();
            const QString rootAbs = rootInfo.absoluteFilePath();
            if (linkTarget.startsWith(rootAbs + QLatin1Char('/')))
                linkTarget = QDir(rootAbs).relativeFilePath(linkTarget);
            o.insert(QStringLiteral("link"), linkTarget);
        } else if (fi.isDir()) {
            o.insert(QStringLiteral("type"), QStringLiteral("dir"));
        } else {
            o.insert(QStringLiteral("type"), QStringLiteral("file"));
            o.insert(QStringLiteral("size"), fi.size());
            o.insert(QStringLiteral("sha256"), sha256OfFile(fi.absoluteFilePath()));
        }
        o.insert(QStringLiteral("mode"), QString::number(static_cast<uint>(fi.permissions()), 8));
        o.insert(QStringLiteral("mtime"), static_cast<qint64>(fi.lastModified().toSecsSinceEpoch()));
        arr.append(o);
    }

    // 排序保证可比对
    std::vector<QJsonValue> v;
    v.reserve(static_cast<size_t>(arr.size()));
    for (const auto &x : arr)
        v.push_back(x);
    std::sort(v.begin(), v.end(), [](const QJsonValue &a, const QJsonValue &b) {
        return a.toObject().value(QStringLiteral("path")).toString()
             < b.toObject().value(QStringLiteral("path")).toString();
    });
    QJsonArray sorted;
    for (const auto &x : v)
        sorted.append(x);
    return sorted;
}

QJsonArray archiveDataEntries(const ArchiveData &data)
{
    QJsonArray arr;
    for (auto it = data.mapFileEntry.constBegin(); it != data.mapFileEntry.constEnd(); ++it) {
        const FileEntry &e = it.value();
        QJsonObject o;
        o.insert(QStringLiteral("name"), it.key());
        o.insert(QStringLiteral("fullPath"), e.strFullPath);
        o.insert(QStringLiteral("isDir"), e.isDirectory);
        o.insert(QStringLiteral("size"), e.qSize);
        o.insert(QStringLiteral("mtime"), static_cast<qint64>(e.uLastModifiedTime));
        o.insert(QStringLiteral("alias"), e.strAlias);
        o.insert(QStringLiteral("index"), e.iIndex);
        arr.append(o);
    }
    return arr;
}

QJsonArray rootEntries(const ArchiveData &data)
{
    QJsonArray arr;
    for (const FileEntry &e : data.listRootEntry) {
        QJsonObject o;
        o.insert(QStringLiteral("fullPath"), e.strFullPath);
        o.insert(QStringLiteral("isDir"), e.isDirectory);
        o.insert(QStringLiteral("size"), e.qSize);
        arr.append(o);
    }
    return arr;
}

// Query 自动应答：把"人点对话框"变成确定性策略
void respondToQuery(Query *query, const QString &policy, Observation &obs)
{
    QJsonObject rec;
    int response = -1;

    if (auto *ow = qobject_cast<OverwriteQuery *>(query)) {
        rec.insert(QStringLiteral("type"), QStringLiteral("Overwrite"));
        rec.insert(QStringLiteral("file"), ow->property("filename").toString());
        if (policy == QLatin1String("skip"))
            response = Result_Skip;
        else if (policy == QLatin1String("cancel"))
            response = Result_Cancel;
        else
            response = Result_OverwriteAll;
    } else if (auto *et = qobject_cast<ExtractToQuery *>(query)) {
        rec.insert(QStringLiteral("type"), QStringLiteral("ExtractTo"));
        response = (policy == QLatin1String("cancel")) ? Result_Cancel : Result_Overwrite;
    } else if (auto *pw = qobject_cast<PasswordNeededQuery *>(query)) {
        rec.insert(QStringLiteral("type"), QStringLiteral("PasswordNeeded"));
        rec.insert(QStringLiteral("file"), pw->property("fileName").toString());
        // 无注入缝：只能表达"取消"。密码注入需在 Query 上补一个测试钩子（见方案文档）。
        query->setResponse(policy != QLatin1String("overwrite"));
        rec.insert(QStringLiteral("response"), policy != QLatin1String("overwrite"));
        obs.queries.append(rec);
        return;
    } else if (qobject_cast<LoadCorruptQuery *>(query)) {
        rec.insert(QStringLiteral("type"), QStringLiteral("LoadCorrupt"));
        response = Result_Readonly;
    } else if (query) {
        rec.insert(QStringLiteral("type"), QString::fromLatin1(query->metaObject()->className()));
        response = 1;
    }

    rec.insert(QStringLiteral("response"), response);
    if (response >= 0)
        query->setResponse(response);
    obs.queries.append(rec);
}

ReadWriteArchiveInterface *asWrite(ReadOnlyArchiveInterface *iface)
{
    return dynamic_cast<ReadWriteArchiveInterface *>(iface);
}

} // namespace

int main(int argc, char *argv[])
{
    // Query 里含 DTK 控件构造，需要 QApplication；offscreen 保证无显示环境可跑
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("conformance"));

    QCommandLineParser parser;
    parser.addHelpOption();
    const QCommandLineOption optBackend(QStringLiteral("backend"), QStringLiteral("后端标签（仅用于记录）"), QStringLiteral("id"));
    const QCommandLineOption optSo(QStringLiteral("so"), QStringLiteral("插件 .so 路径"), QStringLiteral("path"));
    const QCommandLineOption optPluginDir(QStringLiteral("plugin-dir"), QStringLiteral("插件扫描目录（op=scan）"), QStringLiteral("path"));
    const QCommandLineOption optMeta(QStringLiteral("meta"), QStringLiteral("插件元数据 JSON（默认取 .so 同目录唯一 kerfuffle_*.json）"), QStringLiteral("path"));
    const QCommandLineOption optOp(QStringLiteral("op"), QStringLiteral("list|test|extract|add|delete|rename"), QStringLiteral("op"));
    const QCommandLineOption optSample(QStringLiteral("sample"), QStringLiteral("被测压缩包"), QStringLiteral("path"));
    const QCommandLineOption optTarget(QStringLiteral("target"), QStringLiteral("解压目标目录"), QStringLiteral("dir"));
    const QCommandLineOption optInput(QStringLiteral("input"), QStringLiteral("待压缩文件（可多次）"), QStringLiteral("path"));
    const QCommandLineOption optArchive(QStringLiteral("archive"), QStringLiteral("输出的压缩包路径"), QStringLiteral("path"));
    const QCommandLineOption optEmit(QStringLiteral("emit"), QStringLiteral("输出格式 mime"), QStringLiteral("mime"), QStringLiteral("application/zip"));
    const QCommandLineOption optRespond(QStringLiteral("respond"), QStringLiteral("Query 策略 overwrite|skip|cancel"), QStringLiteral("policy"), QStringLiteral("overwrite"));
    const QCommandLineOption optLevel(QStringLiteral("level"), QStringLiteral("压缩等级"), QStringLiteral("n"), QStringLiteral("6"));
    const QCommandLineOption optTimeout(QStringLiteral("timeout"), QStringLiteral("异步等待超时毫秒"), QStringLiteral("ms"), QStringLiteral("60000"));
    const QCommandLineOption optKeep(QStringLiteral("keep"), QStringLiteral("保留临时解压目录"));
    const QCommandLineOption optEntry(QStringLiteral("entry"), QStringLiteral("压缩包内条目名（delete/rename/部分解压用，可多次）"), QStringLiteral("name"));
    const QCommandLineOption optPartial(QStringLiteral("partial"), QStringLiteral("部分解压：只解 --entry 指定的条目（bAllExtract=false）"));
    const QCommandLineOption optAlias(QStringLiteral("alias"), QStringLiteral("重命名后的名字"), QStringLiteral("name"));
    const QCommandLineOption optPrefill(QStringLiteral("prefill"), QStringLiteral("解压前预置到目标目录的文件（触发覆盖询问）"), QStringLiteral("path"));
    const QCommandLineOption optOut(QStringLiteral("out"), QStringLiteral("输出 JSON 文件"), QStringLiteral("path"));
    parser.addOptions({optBackend, optSo, optPluginDir, optMeta, optOp, optSample, optTarget, optInput, optArchive,
                       optEmit, optRespond, optLevel, optTimeout, optKeep, optPrefill, optEntry, optPartial, optAlias, optOut});
    parser.process(app);

    const QString backend = parser.value(optBackend);
    // KPluginLoader 不做 cwd 解析：统一转绝对路径
    const QString soPath  = QFileInfo(parser.value(optSo)).absoluteFilePath();
    const QString op      = parser.value(optOp);
    const QString sample  = parser.value(optSample);
    const QString policy  = parser.value(optRespond);

    QJsonObject out;
    out.insert(QStringLiteral("schema"), 1);
    QJsonObject caseObj;
    caseObj.insert(QStringLiteral("backend"), backend);
    caseObj.insert(QStringLiteral("op"), op);
    caseObj.insert(QStringLiteral("sample"), sample);
    caseObj.insert(QStringLiteral("respond"), policy);
    caseObj.insert(QStringLiteral("level"), parser.value(optLevel).toInt());
    out.insert(QStringLiteral("case"), caseObj);

    auto fail = [&](const QString &msg) {
        QJsonObject err;
        err.insert(QStringLiteral("error"), msg);
        out.insert(QStringLiteral("result"), err);
        const QByteArray json = QJsonDocument(out).toJson(QJsonDocument::Indented);
        fwrite(json.constData(), 1, static_cast<size_t>(json.size()), stdout);
        return 2;
    };

    if (op == QLatin1String("scan")) {
        // 目录扫描：验证 KPluginLoader::findPlugins + KPluginMetaData
        const QString dir = parser.value(optPluginDir);
        if (dir.isEmpty())
            return fail(QStringLiteral("scan 需要 --plugin-dir"));

        QJsonArray arr;
        const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(dir);
        for (const KPluginMetaData &md : plugins) {
            QJsonObject item;
            item.insert(QStringLiteral("id"), md.pluginId());
            item.insert(QStringLiteral("name"), md.name());
            item.insert(QStringLiteral("mimeCount"), md.mimeTypes().size());
            item.insert(QStringLiteral("file"), QFileInfo(md.fileName()).fileName());
            arr.append(item);
        }
        QJsonObject result;
        result.insert(QStringLiteral("count"), plugins.size());
        out.insert(QStringLiteral("plugins"), arr);
        out.insert(QStringLiteral("result"), result);
        const QByteArray json = QJsonDocument(out).toJson(QJsonDocument::Indented);
        fwrite(json.constData(), 1, static_cast<size_t>(json.size()), stdout);
        return 0;
    }

    if (soPath.isEmpty() || op.isEmpty())
        return fail(QStringLiteral("缺少 --so 或 --op"));
    if ((op == QLatin1String("list") || op == QLatin1String("test") || op == QLatin1String("extract"))
            && sample.isEmpty())
        return fail(QStringLiteral("该操作需要 --sample"));
    if (!QFileInfo::exists(soPath))
        return fail(QStringLiteral("插件不存在: ") + soPath);

    // 与 UiTools::createInterface 完全一致的构造参数。
    // 元数据取插件同目录的 kerfuffle_*.json —— 内容与 .so 内嵌的 "MetaData" 完全一致，
    // 但不依赖 QPluginLoader 能否 dlopen（未安装环境下更可靠）。
    QString metaPath = parser.value(optMeta);
    if (metaPath.isEmpty()) {
        QDir soDir = QFileInfo(soPath).absoluteDir();
        const QStringList jsons = soDir.entryList({QStringLiteral("kerfuffle_*.json")}, QDir::Files);
        if (jsons.size() == 1) {
            metaPath = soDir.absoluteFilePath(jsons.first());
        } else {
            // 构建树里没有 JSON：按插件目录名回源仓库查找
            const QString pluginDirName = soDir.dirName();
            QDirIterator it(QStringLiteral(REPO_SOURCE_DIR) + QStringLiteral("/3rdparty"),
                            {QStringLiteral("kerfuffle_*.json")}, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                const QString cand = it.next();
                if (QFileInfo(cand).absolutePath().endsWith(QLatin1Char('/') + pluginDirName)) {
                    metaPath = cand;
                    break;
                }
            }
        }
    }
    KPluginMetaData meta = metaPath.isEmpty() ? KPluginMetaData(soPath) : KPluginMetaData(metaPath);
    if (!meta.isValid())
        return fail(QStringLiteral("插件元数据无效: ") + (metaPath.isEmpty() ? soPath : metaPath));
    out.insert(QStringLiteral("pluginId"), meta.pluginId());
    out.insert(QStringLiteral("pluginName"), meta.name());

    QTemporaryDir tmpDir;
    ExtractionOptions extractOptions;
    QString targetPath = parser.value(optTarget);
    if (targetPath.isEmpty()) {
        if (!tmpDir.isValid())
            return fail(QStringLiteral("无法创建临时目录"));
        targetPath = tmpDir.path() + QStringLiteral("/out");
        QDir().mkpath(targetPath);
    }
    const QString prefill = parser.value(optPrefill);
    if (!prefill.isEmpty()) {
        QDir().mkpath(targetPath);
        const QString dst = QDir(targetPath).filePath(QFileInfo(prefill).fileName());
        QFile::remove(dst);
        if (!QFile::copy(prefill, dst))
            return fail(QStringLiteral("预置文件失败: ") + prefill);
    }

    extractOptions.strTargetPath = targetPath;
    extractOptions.bAllExtract = true;
    extractOptions.bExistList = true;
    // 与生产一致：mainwindow 用 archiveData 的大小作为进度分母
    extractOptions.qSize = QFileInfo(sample).size();
    extractOptions.qComressSize = extractOptions.qSize;
    out.insert(QStringLiteral("target"), targetPath);
    out.insert(QStringLiteral("tempTarget"), parser.isSet(optKeep) ? false : true);

    // 读操作：复用生产的 determineMimeType()（含 tar.bz2/tar.7z/crx 等特例）
    // 写操作：直接用 --emit 指定的目标格式
    CustomMimeType cmt;
    if (op == QLatin1String("add")) {
        cmt.m_mimeType = QMimeDatabase().mimeTypeForName(parser.value(optEmit));
    } else {
        cmt = determineMimeType(sample);
    }
    out.insert(QStringLiteral("mimeType"), cmt.name());

    const QVariantList args = {
        QVariant(op == QLatin1String("add") ? parser.value(optArchive) : sample),
        QVariant::fromValue(meta),
        QVariant::fromValue(cmt)
    };

    KPluginLoader loader(soPath);
    KPluginFactory *factory = loader.factory();
    if (!factory)
        return fail(QStringLiteral("插件工厂加载失败: ") + loader.errorString());

    ReadOnlyArchiveInterface *iface = factory->create<ReadOnlyArchiveInterface>(nullptr, args);
    if (!iface)
        return fail(QStringLiteral("接口创建失败（后端可能不支持该格式）"));

    Observation obs;
    obs.async = iface->waitForFinished();

    QObject::connect(iface, &ReadOnlyArchiveInterface::signalprogress, [&obs](double p) {
        obs.progress.push_back(p);
    });
    QObject::connect(iface, &ReadOnlyArchiveInterface::signalCurFileName, [&obs](const QString &n) {
        obs.curFiles.push_back(n);
    });
    QObject::connect(iface, &ReadOnlyArchiveInterface::signalFinished, [&obs](PluginFinishType t) {
        obs.finishedSignals++;
        obs.finish = static_cast<int>(t);
    });
    QObject::connect(iface, &ReadOnlyArchiveInterface::signalCancel, [&obs]() {
        obs.cancelSignals++;
    });
    QObject::connect(iface, &ReadOnlyArchiveInterface::signalQuery, [&obs, policy](Query *q) {
        respondToQuery(q, policy, obs);
    });

    DataManager::get_instance().resetArchiveData();

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, [&]() { obs.waitTimedOut = true; loop.quit(); });
    if (obs.async) {
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalFinished, &loop, &QEventLoop::quit);
        timer.start(parser.value(optTimeout).toInt());
    }

    int ret = 0;
    if (op == QLatin1String("list")) {
        ret = static_cast<int>(iface->list());
    } else if (op == QLatin1String("test")) {
        ret = static_cast<int>(iface->testArchive());
    } else if (op == QLatin1String("extract")) {
        QList<FileEntry> files;
        if (parser.isSet(optPartial)) {
            // 与生产一致：条目由 UI 侧的列表构造，这里直接按 --entry 造出来；
            // 后端内部会先做一次 list（listRootEntry 为空时），再继续解压。
            for (const QString &name : parser.values(optEntry)) {
                FileEntry e;
                e.strFullPath = name;
                e.strFileName = QFileInfo(name).fileName();
                e.isDirectory = name.endsWith(QLatin1Char('/'));
                files << e;
            }
            if (files.isEmpty())
                return fail(QStringLiteral("部分解压需要 --entry"));
            extractOptions.bAllExtract = false;
        }
        if (qEnvironmentVariableIsSet("DC_CONF_DEBUG"))
            qWarning() << "[conf] extract partial" << !extractOptions.bAllExtract
                       << "files" << files.size()
                       << "rootEntry" << DataManager::get_instance().archiveData().listRootEntry.size();
        ret = static_cast<int>(iface->extractFiles(files, extractOptions));
    } else if (op == QLatin1String("add")) {
        ReadWriteArchiveInterface *w = asWrite(iface);
        if (!w)
            return fail(QStringLiteral("该后端不是读写后端，无法执行 add"));
        CompressOptions copts;
        copts.iCompressionLevel = parser.value(optLevel).toInt();
        copts.strCompressionMethod = QString();
        // tar.7z：与生产一致，按目标扩展名判定
        copts.bTar_7z = parser.value(optArchive).endsWith(QLatin1String(".tar.7z"));
        QList<FileEntry> files;
        for (const QString &in : parser.values(optInput)) {
            QFileInfo fi(in);
            FileEntry e;
            e.strFullPath = fi.absoluteFilePath();
            e.strFileName = fi.fileName();
            e.isDirectory = fi.isDir();
            e.qSize = fi.isDir() ? 0 : fi.size();
            e.uLastModifiedTime = static_cast<uint>(fi.lastModified().toSecsSinceEpoch());
            files << e;
        }
        if (files.isEmpty())
            return fail(QStringLiteral("add 需要 --input"));
        ret = static_cast<int>(w->addFiles(files, copts));
    } else if (op == QLatin1String("delete") || op == QLatin1String("rename")) {
        ReadWriteArchiveInterface *w = asWrite(iface);
        if (!w)
            return fail(QStringLiteral("该后端不是读写后端"));
        // 与生产一致：条目来自 list() 的结果，带包内索引
        iface->list();
        const ArchiveData &data = DataManager::get_instance().archiveData();

        QList<FileEntry> entries;
        for (const QString &name : parser.values(optEntry)) {
            FileEntry e = data.mapFileEntry.value(name);
            if (e.strFullPath.isEmpty()) {
                e.strFullPath = name;
                e.strFileName = QFileInfo(name).fileName();
                e.isDirectory = name.endsWith(QLatin1Char('/'));
            }
            e.strAlias = parser.value(optAlias);
            entries << e;
        }
        if (entries.isEmpty())
            return fail(QStringLiteral("需要 --entry"));
        ret = static_cast<int>(op == QLatin1String("delete") ? w->deleteFiles(entries)
                                                             : w->renameFiles(entries));
    } else {
        return fail(QStringLiteral("未知 op: ") + op);
    }

    if (obs.async && !obs.waitTimedOut)
        loop.exec();

    if (!obs.async)
        obs.finish = ret;

    QJsonObject result;
    result.insert(QStringLiteral("finish"), obs.finish);
    result.insert(QStringLiteral("returned"), ret);
    result.insert(QStringLiteral("errorType"), static_cast<int>(iface->errorType()));
    result.insert(QStringLiteral("async"), obs.async);
    result.insert(QStringLiteral("waitTimedOut"), obs.waitTimedOut);
    out.insert(QStringLiteral("result"), result);

    const ArchiveData &data = DataManager::get_instance().archiveData();
    QJsonObject dataObj;
    dataObj.insert(QStringLiteral("qSize"), data.qSize);
    dataObj.insert(QStringLiteral("compressedSize"), static_cast<qint64>(data.qComressSize));
    dataObj.insert(QStringLiteral("comment"), data.strComment);
    dataObj.insert(QStringLiteral("isListEncrypted"), data.isListEncrypted);
    dataObj.insert(QStringLiteral("entryCount"), data.mapFileEntry.size());
    dataObj.insert(QStringLiteral("entries"), archiveDataEntries(data));
    dataObj.insert(QStringLiteral("rootEntries"), rootEntries(data));
    out.insert(QStringLiteral("archiveData"), dataObj);

    bool monotonic = true;
    double prev = -1.0;
    for (double p : obs.progress) {
        if (p + 1e-9 < prev)
            monotonic = false;
        prev = p;
    }
    QJsonObject sig;
    sig.insert(QStringLiteral("progressCount"), obs.progress.size());
    sig.insert(QStringLiteral("progressFirst"), obs.progress.isEmpty() ? -1.0 : obs.progress.first());
    sig.insert(QStringLiteral("progressLast"), obs.progress.isEmpty() ? -1.0 : obs.progress.last());
    sig.insert(QStringLiteral("progressMonotonic"), monotonic);
    sig.insert(QStringLiteral("curFileCount"), obs.curFiles.size());
    QStringList curSample = obs.curFiles;
    if (curSample.size() > 20)
        curSample = curSample.mid(0, 20);
    sig.insert(QStringLiteral("curFiles"), QJsonArray::fromStringList(curSample));
    sig.insert(QStringLiteral("finishedSignals"), obs.finishedSignals);
    sig.insert(QStringLiteral("cancelSignals"), obs.cancelSignals);
    sig.insert(QStringLiteral("queries"), obs.queries);
    out.insert(QStringLiteral("signals"), sig);

    if (op == QLatin1String("extract"))
        out.insert(QStringLiteral("fs"), snapshotTree(targetPath));

    if (op == QLatin1String("add") || op == QLatin1String("delete") || op == QLatin1String("rename")) {
        const QString archivePath = op == QLatin1String("add") ? parser.value(optArchive) : sample;
        QJsonObject produced;
        produced.insert(QStringLiteral("path"), QFileInfo(archivePath).fileName());
        produced.insert(QStringLiteral("size"), QFileInfo(archivePath).size());
        // 用独立的外部工具列出产物，避免用被测后端验证自己
        const QList<QPair<QString, QStringList>> listers = {
            {QStringLiteral("bsdtar"), {QStringLiteral("-tf"), archivePath}},
            {QStringLiteral("tar"),    {QStringLiteral("-tf"), archivePath}},
            {QStringLiteral("unzip"),  {QStringLiteral("-Z1"), archivePath}},
        };
        for (const auto &lister : listers) {
            QProcess p;
            p.start(lister.first, lister.second);
            if (!p.waitForFinished(20000) || p.exitCode() != 0)
                continue;
            QStringList names;
            for (const QString &l : QString::fromUtf8(p.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts))
                names << l.trimmed();
            names.sort();
            produced.insert(QStringLiteral("entries"), QJsonArray::fromStringList(names));
            produced.insert(QStringLiteral("lister"), lister.first);
            break;
        }
        out.insert(QStringLiteral("produced"), produced);
    }

    const QByteArray json = QJsonDocument(out).toJson(QJsonDocument::Indented);
    const QString outPath = parser.value(optOut);
    if (outPath.isEmpty()) {
        fwrite(json.constData(), 1, static_cast<size_t>(json.size()), stdout);
    } else {
        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return fail(QStringLiteral("无法写输出: ") + outPath);
        f.write(json);
        f.close();
        fprintf(stdout, "%s", outPath.toUtf8().constData());
    }

    delete iface;
    return 0;
}
