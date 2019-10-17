#include "compressor_success.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>
#include <DDesktopServices>


Compressor_Success::Compressor_Success(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("压缩成功！");
    InitUI();
    InitConnection();
}

void Compressor_Success::InitUI()
{
    QFont ft;
    ft.setPixelSize(17);
    ft.setBold(true);

    DPalette pa;

    m_compressicon = Utils::renderSVG(":/images/success.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    m_stringinfolabel->setFont(ft);
    pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTitle));
    m_stringinfolabel->setPalette(pa);
    m_stringinfolabel->setText(m_stringinfo);
    m_showfilebutton = new DPushButton();
    m_showfilebutton->setFixedSize(340, 36);
    m_showfilebutton->setText(tr("显示文件"));
    m_showfilebutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_showfilebutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::Background, pa.color(DPalette::Base));
    setPalette(pa);
}

void Compressor_Success::InitConnection()
{
    connect(m_showfilebutton, &DPushButton::clicked, this, &Compressor_Success::showfiledirSlot);
}

void Compressor_Success::showfiledirSlot()
{
    DDesktopServices::showFolder(QUrl(m_path, QUrl::TolerantMode));
    emit sigQuitApp();
}

void Compressor_Success::setstringinfo(QString str)
{
    m_stringinfolabel->setText(str);
}

void Compressor_Success::setCompressPath(QString path)
{
    m_path = path;
}
