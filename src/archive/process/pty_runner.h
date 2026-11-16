// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PTY_RUNNER_H
#define PTY_RUNNER_H

#include "process_runner.h"

#include <QByteArray>
#include <functional>

namespace archive {

class PtyRunner : public ProcessRunner
{
    Q_OBJECT

public:
    explicit PtyRunner(QObject *parent = nullptr)
        : ProcessRunner(parent) {}
    ~PtyRunner() override = default;

    virtual bool start(const QString &program,
                       const QStringList &args,
                       const QString &workDir = QString()) override = 0;

    virtual void writeInput(const QByteArray &data) = 0;
    virtual QByteArray readOutput() = 0;
    virtual bool isPasswordPrompt(const QByteArray &line) const = 0;

    using PasswordCallback = std::function<QByteArray()>;
    void setPasswordCallback(PasswordCallback cb) { m_passwordCb = std::move(cb); }

protected:
    PasswordCallback m_passwordCb;
};

} // namespace archive

#endif // PTY_RUNNER_H
