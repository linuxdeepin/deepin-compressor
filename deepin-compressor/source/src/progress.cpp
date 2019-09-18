#include "progress.h"

#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QVBoxLayout>

Progress::Progress(QWidget *parent)
    : QWidget(parent)
{
    m_filename = "new file.rar";
    m_progressfile = "设计图111.jpg";
    InitUI();
    InitConnection();
}

void Progress::InitUI()
{
    m_compressicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);

    QFont ft;
    ft.setPixelSize(17);
    ft.setBold(true);

    m_filenamelabel = new DLabel();
    m_filenamelabel->setText(m_filename);
    m_filenamelabel->setFont(ft);

    m_progressbar = new DProgressBar();
    m_progressbar->setRange(0,100);
    m_progressbar->setFixedSize(240, 8);
    m_progressbar->setValue(0);
    m_progressbar->setOrientation(Qt::Horizontal);  //水平方向
    m_progressbar->setAlignment(Qt::AlignVCenter);
    m_progressbar->setTextVisible(false);

    m_progressfilelabel = new DLabel();
    m_progressfilelabel->setText(tr("正在计算中..."));

    m_cancelbutton = new DPushButton();
    m_cancelbutton->setFixedSize(340, 36);
    m_cancelbutton->setText(tr("取消"));

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addWidget(m_pixmaplabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_filenamelabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_progressbar, 50 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_progressfilelabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_cancelbutton, 100 , Qt::AlignHCenter | Qt::AlignVCenter);
}

void Progress::InitConnection()
{
    connect(m_cancelbutton, &DPushButton::clicked, this, &Progress::cancelbuttonPressedSlot);
}

void Progress::cancelbuttonPressedSlot()
{
    emit sigCancelPressed();
}

void Progress::setprogress(uint percent)
{
    m_progressbar->setValue(percent);
}

void Progress::setFilename(QString filename)
{
    QFileInfo fileinfo(filename);
    setTypeImage(fileinfo.suffix());
    m_filenamelabel->setText(filename);
}

void Progress::setTypeImage(QString type)
{
    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator()  + "tempfile." + type;

    QTemporaryFile tmpFile(strTemplateName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        icon = provider.icon(QFileInfo(strTemplateName));
    }

    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}

void Progress::setProgressFilename(QString filename)
{
    if(m_type == COMPRESSING)
    {
        m_progressfilelabel->setText(tr("正在压缩") + ": " + filename);
    }
    else
    {
        m_progressfilelabel->setText(tr("正在解压") + ": " + filename);
    }
}

void Progress::settype(COMPRESS_TYPE type)
{
    m_type = type;
}
