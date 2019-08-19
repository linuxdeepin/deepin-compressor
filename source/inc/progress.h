#ifndef PROGRESS_H
#define PROGRESS_H

#include <QWidget>
#include <DFileDialog>
#include <DSuggestButton>
#include <dlabel.h>
#include "utils.h"
#include <QProgressBar>

DWIDGET_USE_NAMESPACE

class Progress: public QWidget
{
    Q_OBJECT
public:
    Progress(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

private:
    DSuggestButton* m_cancelbutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_filenamelabel;
    QProgressBar* m_progressbar;
    DLabel* m_progressfilelabel;
    QString m_progressfile;

    QString m_filename;

signals:
    void  sigCancelPressed();

public slots:
    void cancelbuttonPressedSlot();
};

#endif // PROGRESS_H
