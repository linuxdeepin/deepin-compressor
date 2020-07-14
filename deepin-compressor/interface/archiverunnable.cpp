#include "archiverunnable.h"

ArchiveRunnable::ArchiveRunnable(QObject *parent)
    : QObject(parent)
{

}

ArchiveRunnable::~ArchiveRunnable()
{

}

void ArchiveRunnable::run()
{
  emit readLineSig(m_line);
}

void ArchiveRunnable::setReadLine(const QString &line)
{
   m_line = line;
}
