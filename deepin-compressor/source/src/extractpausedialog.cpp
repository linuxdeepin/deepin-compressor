#include "extractpausedialog.h"
#include <QDebug>
#include <utils.h>

ExtractPauseDialog::ExtractPauseDialog(QWidget *parent):
    DDialog(parent)
{
    initUI();
    initConnect();
}

void ExtractPauseDialog::initUI()
{
    addButton(tr("停止提取"));
    addButton(tr("继续提取"));

    QPixmap pixmap = Utils::renderSVG(":/images/folder-128 .svg", QSize(80, 80));
    DLabel* imagelabel = new DLabel;
    DLabel* strlabel = new DLabel;
    imagelabel->setPixmap(pixmap);
    strlabel->setText(tr("停止提取！当前有提取任务正在进行"));
    addContent(imagelabel, Qt::AlignLeft);
    addContent(strlabel, Qt::AlignRight);

//    moveToCenter();

}

void ExtractPauseDialog::initConnect()
{
    connect(this, &DDialog::buttonClicked, this, &ExtractPauseDialog::clickedSlot);
}

void ExtractPauseDialog::clickedSlot(int index, const QString &text)
{
    emit sigbuttonpress(index);
}

void ExtractPauseDialog::closeEvent(QCloseEvent *event)
{
    emit sigbuttonpress(2);
}

