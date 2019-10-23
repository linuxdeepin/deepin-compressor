#include "extractpausedialog.h"
#include <QBoxLayout>
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


    QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(32, 32));
    setIconPixmap(pixmap);

    DPalette pa;

    DLabel* strlabel = new DLabel;
    pa = DApplicationHelper::instance()->palette(strlabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::WindowText));
    strlabel->setPalette(pa);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::Medium);
    strlabel->setFont(font);
    strlabel->setText(tr("停止提取！"));
    DLabel* strlabel2 = new DLabel;
    pa = DApplicationHelper::instance()->palette(strlabel2);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    strlabel2->setPalette(pa);

    font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setBold(true);
    strlabel2->setFont(font);
    strlabel2->setText(tr("当前有提取任务正在进行"));

    addButton(tr("停止提取"));
    addButton(tr("继续提取"));

    QVBoxLayout* mainlayout = new QVBoxLayout;
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    DWidget* widget = new DWidget;

    widget->setLayout(mainlayout);
    addContent(widget);

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

