#ifndef ARCHIVERUNNABLE_H
#define ARCHIVERUNNABLE_H

#include <QObject>
#include <QRunnable>

class ArchiveRunnable : public QObject
    , public QRunnable
{
    Q_OBJECT
public:
    ArchiveRunnable(QObject *parent = nullptr);
    ~ArchiveRunnable() override;

    void run() override;
    void setReadLine(const QString &line);

signals:
    void readLineSig(const QString &);

private:
    QString m_line;
};

#endif // ARCHIVERUNNABLE_H
