#ifndef BATCHCOMPRESS_H
#define BATCHCOMPRESS_H

#include <QMap>
#include <QObject>
#include <QVector>
#include "queries.h"
#include "batchjobs.h"
#include "archivemodel.h"

class BatchCompress: public BatchJobs
{
    Q_OBJECT

public:

    explicit BatchCompress(QObject *parent = nullptr);

    ~BatchCompress();

    void addCompress(const QStringList &files);
    void setCompressArgs(const QMap<QString, QString> &Args);

    bool doKill() override;
    void start() override;
    void addInput(const QStringList &files);

Q_SIGNALS:
    void batchProgress(KJob *job, ulong progress);
    void batchFilenameProgress(KJob *job, const QString & name);
    void sendCurFile(const QString&  filename);

private Q_SLOTS:
    void forwardProgress(KJob *job, unsigned long percent);
    void slotResult(KJob *job) override;
    void slotStartJob();

    void SlotProgressFile(KJob *job, const QString &name);

private:
    int m_initialJobCount;
    QMap<KJob *, QPair<QString, QString> > m_fileNames;

    QVector<QStringList> m_inputs;
    QMap<QString, QString> m_Args;
};

#endif // BATCHCOMPRESS_H
