

#ifndef COMPRESSPAGE_H
#define COMPRESSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QStackedLayout>
#include <DFileDialog>
#include <DPushButton>
#include <QSettings>
#include <QMessageBox>

#include "fileViewer.h"

DWIDGET_USE_NAMESPACE

class CompressPage : public QWidget
{
    Q_OBJECT

public:
    CompressPage(QWidget *parent = 0);
    ~CompressPage();

signals:
    void sigNextPress();

public slots:
    void onNextPress();
    void onAddfileSlot();
    void onSelectedFilesSlot(const QStringList &files);

private:

    fileViewer *m_fileviewer;
    DPushButton* m_nextbutton;
    QSettings *m_settings;
    QStringList m_filelist;

};

#endif
