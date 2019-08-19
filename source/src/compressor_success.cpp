#include "compressor_success.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>
#include <DDesktopServices>


Compressor_Success::Compressor_Success(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("Compressor Successed!");
    InitUI();
    InitConnection();
}

void Compressor_Success::InitUI()
{
    QFont ft;
    ft.setPointSize(30);
    ft.setBold(true);

    m_compressicon = Utils::renderSVG(":/images/font_unload.svg", QSize(160, 160));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    m_stringinfolabel->setFont(ft);
    m_stringinfolabel->setText(m_stringinfo);
    m_showfilebutton = new DSuggestButton();
    m_showfilebutton->setFixedWidth(260);
    m_showfilebutton->setText(tr("Show File"));

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->setMargin(20);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_showfilebutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
}

void Compressor_Success::InitConnection()
{
    connect(m_showfilebutton, &DSuggestButton::clicked, this, &Compressor_Success::showfiledirSlot);
}

void Compressor_Success::showfiledirSlot()
{
    DDesktopServices::showFolder(QUrl("~/Desktop", QUrl::TolerantMode));
    emit sigQuitApp();
}
