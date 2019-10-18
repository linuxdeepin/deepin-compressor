#include "encodingpage.h"
#include <QVBoxLayout>
#include <QFileIconProvider>
#include <QDir>
#include <QTemporaryFile>
#include <QTextCodec>

EncodingPage::EncodingPage(QWidget *parent)
    : QWidget(parent)
{
    InitUI();
    InitConnection();
}

void EncodingPage::InitUI()
{
    m_fileicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_fileicon);

    QFont ft;
    ft.setPixelSize(17);
    ft.setBold(true);

    m_filenamelabel = new DLabel();
    m_filenamelabel->setText("");
    m_filenamelabel->setFont(ft);


    QStringList encodeList;

    for (int mib : QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        QString encodeName = QString(codec->name()).toUpper();

        if (encodeName != "UTF-8" && !encodeList.contains(encodeName)) {
            encodeList.append(encodeName);
        }
    }

    encodeList.sort();
    encodeList.prepend("UTF-8");

    m_codelabel = new DLabel;
    m_codelabel->setText(tr("文件名编码") + ":");
    m_codebox = new DComboBox;
    m_codebox->setFixedWidth(260);
    m_codebox->setEditable(true);
    m_codebox->addItems(encodeList);

    QHBoxLayout* encodelayout = new QHBoxLayout;
    encodelayout->addStretch();
    encodelayout->addWidget(m_codelabel);
    encodelayout->addWidget(m_codebox);
    encodelayout->addStretch();

    ft.setPixelSize(12);
    ft.setBold(false);
    m_detaillabel = new DLabel;
    m_detaillabel->setText(tr("请提供文件名编码以解压此文档"));

    m_cancelbutton = new DPushButton(tr("取消"));
    m_confirmbutton = new DPushButton(tr("确定"));
    m_cancelbutton->setFixedSize(165, 36);
    m_confirmbutton->setFixedSize(165, 36);
    QHBoxLayout* buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_cancelbutton);
    buttonlayout->addSpacing(10);
    buttonlayout->addWidget(m_confirmbutton);
    buttonlayout->addStretch();


    QVBoxLayout* mainlayout = new QVBoxLayout(this);
//    mainlayout->addSpacing(62);
    mainlayout->addWidget(m_pixmaplabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_filenamelabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(33);
    mainlayout->addLayout(encodelayout);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(m_detaillabel, 10 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(41);
    mainlayout->addLayout(buttonlayout);
    mainlayout->addSpacing(20);

    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::Background, pa.color(DPalette::Base));
    setPalette(pa);
}

void EncodingPage::InitConnection()
{

}

void EncodingPage::setFilename(QString filename)
{
    QFileInfo fileinfo(filename);
    setTypeImage(fileinfo.suffix());
    m_filenamelabel->setText(filename);
    m_codebox->setEditText("Unicode(UTF-8) 世界城市.txt");
}

void EncodingPage::setTypeImage(QString type)
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
