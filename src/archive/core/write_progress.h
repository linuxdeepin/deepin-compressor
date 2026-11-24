#ifndef WRITE_PROGRESS_H
#define WRITE_PROGRESS_H

#include "archive_backend.h"
#include "path_guard.h"

#include <QDirIterator>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QThread>

#include <algorithm>
#include <atomic>

namespace dcarchive {

inline bool waitForWriteResume(const std::atomic_bool &paused, const std::atomic_bool &cancelled)
{
    while (paused && !cancelled)
        QThread::msleep(20);
    return !cancelled;
}

inline qint64 compressionSourceBytes(const AddRequest &request, const QStringList &excluded,
                                     const std::atomic_bool &cancelled)
{
    if (request.sourceBytes > 0)
        return request.sourceBytes;
    const auto isExcluded = [&](const QFileInfo &info) {
        return std::any_of(excluded.cbegin(), excluded.cend(), [&](const QString &path) {
            return archivePathWithin(info.absoluteFilePath(), path);
        });
    };
    qint64 total = 0;
    for (const auto &entry : request.entries) {
        if (cancelled)
            break;
        const QFileInfo source(entry.path);
        if (source.isSymLink() || isExcluded(source))
            continue;
        if (source.isFile()) {
            total += source.size();
        } else if (source.isDir()) {
            QDirIterator it(source.absoluteFilePath(), QDir::Files | QDir::Hidden | QDir::NoSymLinks,
                            QDirIterator::Subdirectories);
            while (!cancelled && it.hasNext()) {
                it.next();
                if (!isExcluded(it.fileInfo()))
                    total += it.fileInfo().size();
            }
        }
    }
    return total;
}

class WriteProgress
{
public:
    explicit WriteProgress(BackendObserver &observer, qint64 totalBytes = 0)
        : m_observer(observer), m_totalBytes(totalBytes) {}

    void addBytes(qint64 bytes) { setBytes(m_processedBytes + bytes); }

    void setBytes(qint64 bytes)
    {
        if (bytes <= m_processedBytes || m_totalBytes <= 0)
            return;
        m_processedBytes = bytes;
        report(100.0 * bytes / m_totalBytes);
    }

    void report(double percentage)
    {
        percentage = qMin(99.0, percentage);
        if (percentage <= m_last || (percentage < 99.0 && m_timer.isValid() && m_timer.elapsed() < 100))
            return;
        m_last = percentage;
        m_timer.start();
        m_observer.onProgress({percentage, m_processedBytes, m_totalBytes});
    }

    void toolOutput(const QString &chunk)
    {
        static const QRegularExpression percent(QStringLiteral("(?:^|[\\x08\\r\\n]) *([0-9]{1,3})%"));
        const QString output = m_tail + chunk;
        auto matches = percent.globalMatch(output);
        while (matches.hasNext())
            report(matches.next().captured(1).toInt());
        const qsizetype separator = qMax(output.lastIndexOf(QLatin1Char('\b')),
                qMax(output.lastIndexOf(QLatin1Char('\r')), output.lastIndexOf(QLatin1Char('\n'))));
        m_tail = separator >= 0 && output.size() - separator <= 6 ? output.mid(separator) : QString();
    }

private:
    BackendObserver &m_observer;
    qint64 m_totalBytes;
    qint64 m_processedBytes = 0;
    double m_last = 0;
    QElapsedTimer m_timer;
    QString m_tail;
};

} // namespace dcarchive

#endif
