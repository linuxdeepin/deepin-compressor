#ifndef EXTRACTPAUSEDIALOG_H
#define EXTRACTPAUSEDIALOG_H

#include <DPushButton>
#include <DDialog>
#include <DLabel>

DWIDGET_USE_NAMESPACE


class ExtractPauseDialog: public DDialog
{
    Q_OBJECT
public:
    explicit ExtractPauseDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

public slots:
    void clickedSlot(int index, const QString &text);
    void closeEvent(QCloseEvent *event) override;

signals:
    void sigbuttonpress(int index);
};

#endif // EXTRACTPAUSEDIALOG_H
