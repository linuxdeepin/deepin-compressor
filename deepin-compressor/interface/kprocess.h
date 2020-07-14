/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KPROCESS_H
#define KPROCESS_H

#include "kcoreaddons_export.h"

#include <QProcess>

class KProcessPrivate;

class KCOREADDONS_EXPORT KProcess : public QProcess
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KProcess)

public:

    /**
     * Modes in which the output channels can be opened.
     */
    enum OutputChannelMode {
        SeparateChannels = QProcess::SeparateChannels,
        MergedChannels = QProcess::MergedChannels,
        ForwardedChannels = QProcess::ForwardedChannels,
        OnlyStdoutChannel = QProcess::ForwardedErrorChannel,
        OnlyStderrChannel = QProcess::ForwardedOutputChannel
    };

    explicit KProcess(QObject *parent = nullptr);
    ~KProcess();

    void setOutputChannelMode(OutputChannelMode mode);
    OutputChannelMode outputChannelMode() const;
    void setNextOpenMode(QIODevice::OpenMode mode);
    void setEnv(const QString &name, const QString &value, bool overwrite = true);
    void unsetEnv(const QString &name);
    void clearEnvironment();
    void setProgram(const QString &exe, const QStringList &args = QStringList());
    void setProgram(const QStringList &argv);
    KProcess &operator<<(const QString &arg);
    KProcess &operator<<(const QStringList &args);
    void clearProgram();
//    void setShellCommand(const QString &cmd);
    QStringList program() const;
    void start();
    int execute(int msecs = -1);
    static int execute(const QString &exe, const QStringList &args = QStringList(), int msecs = -1);
    static int execute(const QStringList &argv, int msecs = -1);
    int startDetached();
    static int startDetached(const QString &exe, const QStringList &args = QStringList());
    static int startDetached(const QStringList &argv);
    int pid() const;

protected:
    KProcess(KProcessPrivate *d, QObject *parent);
    KProcessPrivate *const d_ptr;

private:
    // hide those
    using QProcess::setReadChannelMode;
    using QProcess::readChannelMode;
    using QProcess::setProcessChannelMode;
    using QProcess::processChannelMode;
};

#endif

