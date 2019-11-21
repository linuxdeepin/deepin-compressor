#ifndef CLIPLUGIN_H
#define CLIPLUGIN_H

#include "../../deepin-compressor/interface/cliinterface.h"
#include "../../deepin-compressor/interface/kpluginfactory.h"

class CliPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_cliunarchiver.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit CliPluginFactory();
    ~CliPluginFactory();
};


class CliPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit CliPlugin(QObject *parent, const QVariantList &args);
    ~CliPlugin() override;

    bool list() override;
    bool extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;
    void resetParsing() override;
    bool readListLine(const QString &line) override;
    bool readExtractLine(const QString &line) override;
    bool isPasswordPrompt(const QString &line) override;

    /**
     * Fill the lsar's json output all in once (useful for unit testing).
     */
    void setJsonOutput(const QString &jsonOutput);

protected Q_SLOTS:
    void readStdout(bool handleAll = false) override;

protected:

    bool handleLine(const QString &line) override;

private Q_SLOTS:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus) override;

private:
    void setupCliProperties();
    void readJsonOutput();

    QString m_jsonOutput;
};

#endif // CLIPLUGIN_H
