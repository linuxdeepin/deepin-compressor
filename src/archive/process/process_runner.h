// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <functional>

namespace archive {

class ProcessRunner : public QObject
{
    Q_OBJECT

public:
    explicit ProcessRunner(QObject *parent = nullptr)
        : QObject(parent) {}
    ~ProcessRunner() override = default;

    virtual bool start(const QString &program,
                       const QStringList &args,
                       const QString &workDir = QString()) = 0;

    virtual bool waitForFinished(int timeoutMs = -1) = 0;
    virtual QByteArray readAllStandardOutput() = 0;
    virtual QByteArray readAllStandardError() = 0;
    virtual int exitCode() const = 0;
    virtual bool isRunning() const = 0;

    virtual void setTimeout(int timeoutMs) = 0;
    virtual int timeout() const = 0;

    virtual void cancel() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    using OutputCallback = std::function<void(const QByteArray &)>;
    void setOutputCallback(OutputCallback cb) { m_outputCb = std::move(cb); }

signals:
    void finished(int exitCode, bool cancelled);
    void errorOccurred(const QString &errorString);
    void outputReady(const QByteArray &data);

protected:
    OutputCallback m_outputCb;
};

} // namespace archive

#endif // PROCESS_RUNNER_H
