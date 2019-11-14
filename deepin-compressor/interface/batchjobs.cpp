#include "batchjobs.h"
#include <QDebug>

//BatchJobsPrivate::BatchJobsPrivate()
//{
//}

//BatchJobsPrivate::~BatchJobsPrivate()
//{
//}



BatchJobs::BatchJobs(QObject *parent)
    : KJob(parent)
{
}

//BatchJobs::BatchJobs(BatchJobsPrivate &dd, QObject *parent)
//    : KJob(dd, parent)
//{
//}

BatchJobs::~BatchJobs()
{
}

bool BatchJobs::addSubjob(KJob *job)
{
//    Q_D(BatchJobs);
    if (job == nullptr || m_subjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    m_subjobs.append(job);
    connect(job, &KJob::result, this, &BatchJobs::slotResult);
    // Forward information from that subjob.
    connect(job, &KJob::infoMessage, this, &BatchJobs::slotInfoMessage);

    return true;
}

bool BatchJobs::removeSubjob(KJob *job)
{
    // remove only Subjobs that are on the list
    if (m_subjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &BatchJobs::slotResult);
        disconnect(job, &KJob::infoMessage, this, &BatchJobs::slotInfoMessage);
        return true;
    }
    return false;
}

bool BatchJobs::hasSubjobs() const
{
    return !m_subjobs.isEmpty();
}

const QList<KJob *> &BatchJobs::subjobs() const
{
    return m_subjobs;
}

void BatchJobs::clearSubjobs()
{
    Q_FOREACH (KJob *job, m_subjobs) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &BatchJobs::slotResult);
        disconnect(job, &KJob::infoMessage, this, &BatchJobs::slotInfoMessage);
    }
    m_subjobs.clear();
}

void BatchJobs::slotResult(KJob *job)
{
    // Did job have an error ?
    if (job->error() && !error()) {
        // Store it in the parent only if first error
        setError(job->error());
        setErrorText(job->errorText());
        // Finish this job
        emitResult();
    }
    // After a subjob is done, we might want to start another one.
    // Therefore do not emitResult
    removeSubjob(job);
}

void BatchJobs::slotInfoMessage(KJob *job, const QString &plain, const QString &rich)
{
    emit infoMessage(job, plain, rich);
}
