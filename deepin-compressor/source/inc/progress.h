#ifndef PROGRESS_H
#define PROGRESS_H

#include <QWidget>
#include <DFileDialog>
#include <DPushButton>
#include <DLabel>
#include "utils.h"
#include <DProgressBar>


DWIDGET_USE_NAMESPACE

class Progress: public QWidget
{
    Q_OBJECT
public:
    Progress(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

private:
    DPushButton* m_cancelbutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_filenamelabel;
    DProgressBar* m_progressbar;
    DLabel* m_progressfilelabel;
    QString m_progressfile;

    QString m_filename;

signals:
    void  sigCancelPressed();

public slots:
    void cancelbuttonPressedSlot();
};

#endif // PROGRESS_H
