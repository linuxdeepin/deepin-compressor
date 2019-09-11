#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <DTitlebar>
#include <ddialog.h>
#include <DPushButton>
#include <DLabel>
#include <dcircleprogress.h>

DWIDGET_USE_NAMESPACE

class ProgressDialog: public DDialog
{
public:
    explicit ProgressDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

    void setCurrentTask(const QString &file);
    void setCurrentFile(const QString &file);
    void setProcess(unsigned long  value);
    void setFinished(const QString &path);


private:
    int m_defaultWidth = 525;
    int m_defaultHeight = 120;

    DLabel* m_tasklable;
    DLabel* m_filelable;
    DCircleProgress* m_circleprogress;

};

#endif // PROGRESSDIALOG_H
