

#ifndef COMPRESSPAGE_H
#define COMPRESSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QStackedLayout>
#include <DFileDialog>
#include <DSuggestButton>

#include "fileViewer.h"

DWIDGET_USE_NAMESPACE

class CompressPage : public QWidget
{
    Q_OBJECT

public:
    CompressPage(QWidget *parent = 0);
    ~CompressPage();

    void addItems(const QStringList &paths);

signals:
    void sigNextPress();

private slots:
    void onNextPress();

private:

    fileViewer *m_fileviewer;
    DSuggestButton* m_nextbutton;
};

#endif
