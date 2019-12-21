#include "singlefileplugin.h"

#include "queries.h"

#include <QFile>
#include <QFileInfo>

//#include <KFilterDev>
//#include <KLocalizedString>

LibSingleFileInterface::LibSingleFileInterface(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
}

LibSingleFileInterface::~LibSingleFileInterface()
{
}

bool LibSingleFileInterface::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)

    QString outputFileName = destinationDirectory;
    if (!destinationDirectory.endsWith(QLatin1Char('/'))) {
        outputFileName += QLatin1Char('/');
    }
    outputFileName += uncompressedFileName();

    outputFileName = overwriteFileName(outputFileName);
    if (outputFileName.isEmpty()) {
        return true;
    }


    QFile outputFile(outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        return false;
    }


    QByteArray dataChunk(1024 * 16, '\0'); // 16Kb


    return true;
}

bool LibSingleFileInterface::list(bool /*isbatch*/)
{

    Archive::Entry *e = new Archive::Entry();
    connect(this, &QObject::destroyed, e, &QObject::deleteLater);
    e->setProperty("fullPath", uncompressedFileName());
    e->setProperty("compressedSize", QFileInfo(filename()).size());
    emit entry(e);

    return true;
}

QString LibSingleFileInterface::overwriteFileName(QString &filename)
{
    QString newFileName(filename);

    while (QFile::exists(newFileName)) {
        OverwriteQuery query(newFileName);

        query.setMultiMode(false);
        emit userQuery(&query);
        query.waitForResponse();

        if ((query.responseCancelled()) || (query.responseSkip())) {
            return QString();
        } else if (query.responseOverwrite()) {
            break;
        } else if (query.responseRename()) {
            newFileName = query.newFilename();
        }
    }

    return newFileName;
}

const QString LibSingleFileInterface::uncompressedFileName() const
{
    QString uncompressedName(QFileInfo(filename()).fileName());

    // Bug 252701: For .svgz just remove the terminal "z".
    if (uncompressedName.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)) {
        uncompressedName.chop(1);
        return uncompressedName;
    }

    for (const QString &extension : qAsConst(m_possibleExtensions)) {

        if (uncompressedName.endsWith(extension, Qt::CaseInsensitive)) {
            uncompressedName.chop(extension.size());
            return uncompressedName;
        }
    }

    return uncompressedName + QStringLiteral(".uncompressed");
}

bool LibSingleFileInterface::testArchive()
{
    return false;
}

