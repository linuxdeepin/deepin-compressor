#include "compressor_fail.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>


Compressor_Fail::Compressor_Fail(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("Sorry, Compressor Failed!");
    m_stringdetail = tr("xxxxxxxxxxxxx");
    InitUI();
    InitConnection();
}

void Compressor_Fail::InitUI()
{
    m_compressicon = Utils::renderSVG(":/images/font_unload.svg", QSize(160, 160));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    m_stringinfolabel->setText(m_stringinfo);
    m_stringinfolabel->setStyleSheet("QLabel { font-size: 18px; }");
    m_stringdetaillabel = new DLabel();
    m_stringdetaillabel->setText(m_stringdetail);
    m_retrybutton = new DPushButton();
    m_retrybutton->setFixedWidth(260);
    m_retrybutton->setText(tr("Retry"));

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->setMargin(20);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringdetaillabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_retrybutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
}

void Compressor_Fail::InitConnection()
{
    connect(m_retrybutton, &DPushButton::clicked, this, &Compressor_Fail::retrySlot);
}

void Compressor_Fail::retrySlot()
{

}
