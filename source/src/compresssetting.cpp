#include "compresssetting.h"

#include <QDebug>
#include <DFileDialog>


static QStringList compress_typelist = {"zip", "7z", "ar", "cbz", "cpio","exe","iso","tar","tar.7z","tar.Z","tar.bz2","tar.gz","tar.lz","tar.lzma","tar.lzo","tar.xz"};

CompressSetting::CompressSetting(QWidget* parent)
    :QWidget(parent)
{
    m_splitnum = 1;
    InitUI();
    InitConnection();
}

CompressSetting::~CompressSetting()
{

}

void CompressSetting::InitUI()
{
    DLabel* namelable = new DLabel(tr("File Name") + ":");
    DLabel* savetolable = new DLabel(tr("Save to") + ":");
    m_nextbutton = new DSuggestButton(tr("NEXT"));
    m_nextbutton->setFixedWidth(260);
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_nextbutton);
    buttonlayout->addStretch();

    QWidget* leftwidget = new QWidget();
    m_compressicon = Utils::renderSVG(":/images/font_unload.svg", QSize(160, 160));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_compresstype = new QComboBox();
    m_compresstype->setFixedSize(80, 40);
    m_compresstype->addItems(compress_typelist);

    m_filename = new DLineEdit();
    m_filename->setFixedWidth(230);
    m_filename->setText(tr("untitled file"));
    m_savepath = new DLineEdit();
    m_savepath->setText("~/Desktop");
    m_savepath->setFixedWidth(230);
    m_pathbutton = new Lib_Edit_Button(m_savepath);

    DLabel* moresetlabel = new DLabel(tr("Advanced Options"));
    m_moresetbutton = new DSwitchButton();
    m_moresetlayout = new QHBoxLayout();
    m_moresetlayout->addWidget(moresetlabel,0 , Qt::AlignLeft);
    m_moresetlayout->addWidget(m_moresetbutton,1 , Qt::AlignRight);


    m_encryptedlabel = new DLabel(tr("Encrypted File"));
    m_password = new DPasswordEdit();
    m_encryptedfilelistlabel = new DLabel(tr("Encrypted file list") + ":");
    m_file_secret = new DSwitchButton();
    m_file_secretlayout = new QHBoxLayout();
    m_file_secretlayout->addWidget(m_encryptedfilelistlabel, 0 , Qt::AlignLeft);
    m_file_secretlayout->addWidget(m_file_secret, 1 , Qt::AlignRight);
    m_splitcompress = new DLabel(tr("Separate compression"));
    m_splitlayout = new QHBoxLayout();
    m_splitnumedit = new DLineEdit();
    m_splitnumedit->setValidator(new QIntValidator(D_COMPRESS_SPLIT_MIN, D_COMPRESS_SPLIT_MIX, this));
    m_splitnumedit->setText(QString::number(m_splitnum));
    m_plusbutton = new DSuggestButton();
//    m_plusbutton->setText("+");
    m_minusbutton = new DSuggestButton();
//    m_minusbutton->setText("-");
    m_splitlayout->addWidget(m_splitnumedit);
    m_splitlayout->addWidget(m_plusbutton);
    m_splitlayout->addWidget(m_minusbutton);
    m_splitlayout->setStretch(0,4);
    m_splitlayout->setStretch(1,1);
    m_splitlayout->setStretch(2,1);


    QVBoxLayout *typeLayout = new QVBoxLayout;
    QHBoxLayout *compresstype = new QHBoxLayout;
    compresstype->addStretch();
    compresstype->addWidget(m_compresstype);
    compresstype->addStretch();
    typeLayout->addStretch();
    typeLayout->addWidget(m_pixmaplabel);
    typeLayout->addLayout(compresstype);
    typeLayout->addStretch();
    leftwidget->setLayout(typeLayout);

    m_fileLayout = new QVBoxLayout;
    m_fileLayout->addStretch();
    m_fileLayout->addWidget(namelable);
    m_fileLayout->addWidget(m_filename);
    m_fileLayout->addStretch();
    m_fileLayout->addWidget(savetolable);
    m_fileLayout->addWidget(m_savepath);
    m_fileLayout->addStretch();



    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->addStretch();
    infoLayout->addWidget(leftwidget);
    infoLayout->addStretch();
    infoLayout->addLayout(m_fileLayout);
    infoLayout->addStretch();




    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(20);
    mainLayout->addStretch();
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonlayout);


}
void CompressSetting::InitConnection()
{
    connect(m_pathbutton, &DSuggestButton::clicked, this, &CompressSetting::onPathButoonClicked);
    connect(m_nextbutton, &DSuggestButton::clicked, this, &CompressSetting::onNextButoonClicked);
    connect(m_moresetbutton, &DSwitchButton::checkedChanged, this, &CompressSetting::onAdvanceButtonClicked);
    connect(m_plusbutton, &DSuggestButton::clicked, this, &CompressSetting::onPlusButoonClicked);
    connect(m_minusbutton, &DSuggestButton::clicked, this, &CompressSetting::onLessButoonClicked);
}


void CompressSetting::onPathButoonClicked()
{
    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setDirectory("~/Desktop");

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    QList<QUrl> pathlist = dialog.selectedUrls();

    QString curpath = pathlist.at(0).toLocalFile();
    m_savepath->setText(curpath);

}

void CompressSetting::onNextButoonClicked()
{
    if(true == m_pathbutton->isVisible())
    {
        m_pathbutton->setVisible(false);
        m_fileLayout->addLayout(m_moresetlayout);
        m_nextbutton->setText(tr("Compress"));
    }
    else {

    }
}

void CompressSetting::onAdvanceButtonClicked(bool status)
{
    if(status)
    {
        m_encryptedlabel->setVisible(true);
        m_password->setVisible(true);
        m_splitcompress->setVisible(true);
        m_encryptedfilelistlabel->setVisible(true);
        m_file_secret->setVisible(true);
        m_splitcompress->setVisible(true);
        m_splitnumedit->setVisible(true);
        m_plusbutton->setVisible(true);
        m_minusbutton->setVisible(true);
        m_fileLayout->addWidget(m_encryptedlabel);
        m_fileLayout->addWidget(m_password);
        m_fileLayout->addLayout(m_file_secretlayout);
        m_fileLayout->addWidget(m_splitcompress);
        m_fileLayout->addLayout(m_splitlayout);
    }
    else {

        m_fileLayout->removeWidget(m_encryptedlabel);
        m_fileLayout->removeWidget(m_password);
        m_fileLayout->removeItem(m_file_secretlayout);
        m_fileLayout->removeWidget(m_splitcompress);
        m_fileLayout->removeItem(m_splitlayout);
        m_encryptedlabel->setVisible(false);
        m_password->setVisible(false);
        m_splitcompress->setVisible(false);
        m_encryptedfilelistlabel->setVisible(false);
        m_file_secret->setVisible(false);
        m_splitcompress->setVisible(false);
        m_splitnumedit->setVisible(false);
        m_plusbutton->setVisible(false);
        m_minusbutton->setVisible(false);
    }
}


void CompressSetting::onPlusButoonClicked()
{
    if(m_splitnum < D_COMPRESS_SPLIT_MIX)
    {
        m_splitnum ++;
        m_splitnumedit->setText(QString::number(m_splitnum));
    }
}

void CompressSetting::onLessButoonClicked()
{
    if(m_splitnum > D_COMPRESS_SPLIT_MIN)
    {
        m_splitnum --;
        m_splitnumedit->setText(QString::number(m_splitnum));
    }
}
