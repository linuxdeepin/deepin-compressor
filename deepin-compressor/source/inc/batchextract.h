#ifndef BATCHEXTRACT_H
#define BATCHEXTRACT_H


#include <QMap>
#include <QObject>
#include <QVector>
#include "queries.h"
#include "batchjobs.h"
#include "archivemodel.h"


class BatchExtract : public BatchJobs
{
    Q_OBJECT

public:

    explicit BatchExtract(QObject* parent = nullptr);

    ~BatchExtract();

    void addExtraction(const QUrl& url);

    bool doKill() override;
    void start() override;
    bool autoSubfolder() const;
    void setAutoSubfolder(bool value);
    void addInput(const QUrl& url);
    bool showExtractDialog();
    QString destinationFolder() const;
    void setDestinationFolder(const QString& folder);
    bool openDestinationAfterExtraction() const;
    void setOpenDestinationAfterExtraction(bool value);
    bool preservePaths() const;
    void setPreservePaths(bool value);

private Q_SLOTS:
    void forwardProgress(KJob *job, unsigned long percent);
    void showFailedFiles();
    void slotResult(KJob *job) override;
    void slotUserQuery(Query *query);
    void slotStartJob();

    void SlotProgressFile(KJob* job, const QString & name);

private:
    int m_initialJobCount;
    QMap<KJob*, QPair<QString, QString> > m_fileNames;
    bool m_autoSubfolder;

    QVector<QUrl> m_inputs;
    QString m_destinationFolder;
    QStringList m_failedFiles;
    bool m_preservePaths;
    bool m_openDestinationAfterExtraction;
};

#endif // BATCHEXTRACT_H
