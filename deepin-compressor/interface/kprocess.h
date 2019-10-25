
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
    ~KProcess() override;

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

