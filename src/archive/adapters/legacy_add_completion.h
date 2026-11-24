#ifndef LEGACY_ADD_COMPLETION_H
#define LEGACY_ADD_COMPLETION_H

#include "archiveinterface.h"

#include <QEventLoop>
#include <QTimer>

namespace dcarchive {

inline PluginFinishType awaitLegacyAdd(ReadWriteArchiveInterface &backend,
                                       const QList<FileEntry> &files,
                                       const CompressOptions &options)
{
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    bool finished = false;
    PluginFinishType result = PFT_Error;
    QObject::connect(&backend, &ReadOnlyArchiveInterface::signalFinished, &loop,
                     [&](PluginFinishType type) {
        result = type;
        finished = true;
        loop.quit();
    });

    const PluginFinishType start = backend.addFiles(files, options);
    if (start != PFT_Nomral)
        return start;
    if (!finished) {
        timeout.start(30 * 60 * 1000);
        loop.exec();
    }
    if (!finished)
        backend.doKill();
    return result;
}

} // namespace dcarchive

#endif // LEGACY_ADD_COMPLETION_H
