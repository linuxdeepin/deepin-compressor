#ifndef BATCHCOMPRESS_H
#define BATCHCOMPRESS_H

#include "batchjobs.h"

#include <QMap>
#include <QVector>

class BatchCompress: public BatchJobs
{
    Q_OBJECT
public:

    explicit BatchCompress(QObject *parent = nullptr);
    ~BatchCompress()override
    {
        m_initialJobCount = 0;
        m_inputs.clear();
        if (m_Args) {
            m_Args->clear();
            delete m_Args;
            m_Args = nullptr;
        }
    }
    void addCompress(const QStringList &files);
    void setCompressArgs(QMap<QString, QString> Args);

    bool doKill() override;
    void start() override;
    void addInput(const QStringList &files);

protected:
    void clearSubjobs();

Q_SIGNALS:
    void batchProgress(KJob *job, ulong progress);
    void batchFilenameProgress(KJob *job, const QString &name);
    void sendCurFile(const QString  &filename);

private Q_SLOTS:
    void forwardProgress(KJob *job, unsigned long percent);
    void slotResult(KJob *job) override;
    void slotStartJob();

    void SlotProgressFile(KJob *job, const QString &name);
    void SlotCreateJobFinished(KJob *job);
private:
    int m_initialJobCount;
//    QMap<KJob *, QPair<QString, QString> > m_fileNames;

    QVector<QStringList> m_inputs;
    QMap<QString, QString> *m_Args = nullptr;
};

#endif // BATCHCOMPRESS_H
