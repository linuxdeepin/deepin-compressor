#include "compressor_fail.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>


Compressor_Fail::Compressor_Fail(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("抱歉，解压失败！");
    m_stringdetail = tr("压缩文件已损坏");
    InitUI();
    InitConnection();
}

void Compressor_Fail::InitUI()
{

    QFont ft;
    ft.setPixelSize(17);
    ft.setBold(true);

    DPalette pa;
    pa.setColor(DPalette::WindowText,QColor(0, 26, 46));

    m_compressicon = Utils::renderSVG(":/images/fail.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    m_stringinfolabel->setText(m_stringinfo);
    m_stringinfolabel->setFont(ft);
    m_stringinfolabel->setPalette(pa);
    m_stringdetaillabel = new DLabel();
    ft.setBold(false);
    pa.setColor(DPalette::WindowText,QColor(82, 106, 127));
    m_stringdetaillabel->setText(m_stringdetail);
    m_retrybutton = new DPushButton();
    m_retrybutton->setFixedSize(340, 36);
    m_retrybutton->setText(tr("重试"));

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringdetaillabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_retrybutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    pa.setColor(DPalette::Background,QColor(255, 255, 255));
    setPalette(pa);
}

void Compressor_Fail::InitConnection()
{
    connect(m_retrybutton, &DPushButton::clicked, this, &Compressor_Fail::sigFailRetry);
}

void Compressor_Fail::setFailStr(const QString& str)
{
    m_stringinfo = str;
    m_stringinfolabel->setText(m_stringinfo);
}

void Compressor_Fail::setFailStrDetail(const QString& str)
{
    m_stringdetail = str;
    m_stringdetaillabel->setText(m_stringdetail);
}
