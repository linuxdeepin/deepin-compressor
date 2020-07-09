#ifndef BATCHJOBS_H
#define BATCHJOBS_H

#include "archivejob.h"
#include "archivejob_p.h"

//class BatchJobs;

//class  BatchJobsPrivate: public KJobPrivate
//{
//public:
//    BatchJobsPrivate();
//    ~BatchJobsPrivate();

//    QList<KJob *> subjobs;

//    Q_DECLARE_PUBLIC(BatchJobs)
//};

class BatchJobs : public KJob
{
    Q_OBJECT
public:
    explicit BatchJobs(QObject *parent = nullptr);
    virtual ~BatchJobs();

protected:
    virtual bool addSubjob(KJob *job);
    virtual bool removeSubjob(KJob *job);
    bool hasSubjobs() const;
    const QList<KJob *> &subjobs() const;
    void clearSubjobs();

public Q_SLOTS:
    virtual void slotResult(KJob *job);
    virtual void slotInfoMessage(KJob *job, const QString &plain, const QString &rich);

protected:
//    BatchJobs(BatchJobsPrivate &dd, QObject *parent);

private:
    QList<KJob *> m_subjobs;
//    Q_DECLARE_PRIVATE(BatchJobs)
};

#endif // BATCHJOBS_H
