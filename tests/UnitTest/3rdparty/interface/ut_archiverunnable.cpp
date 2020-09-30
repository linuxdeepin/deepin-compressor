#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "archiverunnable.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(ArchiveRunnable_ArchiveRunnable_UT, ArchiveRunnable_ArchiveRunnable_UT001)
{
    ArchiveRunnable *options  = new ArchiveRunnable();
    delete options;
    options = nullptr;
}

TEST(ArchiveRunnable_run_UT, ArchiveRunnable_run_UT001)
{
    ArchiveRunnable *options  = new ArchiveRunnable();
    options->run();
    delete options;
    options = nullptr;
}


TEST(ArchiveRunnable_setReadLine_UT, ArchiveRunnable_setReadLine_UT001)
{
    ArchiveRunnable *options  = new ArchiveRunnable();
    options->setReadLine("dfd");
    delete options;
    options = nullptr;
}

