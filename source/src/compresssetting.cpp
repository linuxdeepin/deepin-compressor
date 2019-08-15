#include "compresssetting.h"

#include <QDebug>
#include <DFileDialog>


static QStringList compress_typelist = {"zip", "7z", "ar", "cbz", "cpio","exe","iso","tar","tar.7z","tar.Z","tar.bz2","tar.gz","tar.lz","tar.lzma","tar.lzo","tar.xz"};

CompressSetting::CompressSetting(QWidget* parent)
    :QWidget(parent)
{

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
    m_compressicon = Utils::renderSVG(":/images/font_unload.svg", QSize(160, 160));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_compresstype = new QComboBox();
    m_compresstype->setFixedHeight(40);
    m_compresstype->addItems(compress_typelist);
    m_filename = new DLineEdit();
    m_filename->setFixedWidth(230);
    m_filename->setText(tr("untitled file"));
    m_savepath = new DLineEdit();
    m_savepath->setText("~/Desktop");
    m_savepath->setFixedWidth(230);
    m_pathbutton = new DSuggestButton();

    DLabel* moresetlabel = new DLabel(tr("Advanced Options"));
    m_moresetbutton = new DSwitchButton();
    m_moresetlayout = new QHBoxLayout();
    m_moresetlayout->addWidget(moresetlabel);
    m_moresetlayout->addStretch();
    m_moresetlayout->addWidget(m_moresetbutton);

    m_encryptedlabel = new DLabel(tr("Encrypted File"));
    m_password = new DPasswordEdit();
    DLabel* encryptedfilelistlabel = new DLabel(tr("Encrypted file list"));
    m_file_secret = new DSwitchButton();
    m_file_secretlayout = new QHBoxLayout();
    m_file_secretlayout->addStretch();
    m_file_secretlayout->addWidget(encryptedfilelistlabel);
    m_file_secretlayout->addWidget(m_file_secret);
    m_file_secretlayout->addStretch();
    m_splitcompress = new DLabel(tr("Separate compression"));
    m_splitlayout = new QHBoxLayout();
    m_splitnumedit = new DLineEdit();
    m_splitnumedit->setValidator(new QIntValidator(1, 10, this));
    m_splitnumedit->setText("1");
    m_plusbutton = new DSuggestButton();
    m_minusbutton = new DSuggestButton();
    m_splitlayout->addWidget(m_splitnumedit);
    m_splitlayout->addStretch();
    m_splitlayout->addWidget(m_plusbutton);
    m_splitlayout->addStretch();
    m_splitlayout->addWidget(m_minusbutton);


    QVBoxLayout *typeLayout = new QVBoxLayout;
    typeLayout->addStretch();
    typeLayout->addWidget(m_pixmaplabel);
    typeLayout->addStretch();
    typeLayout->addWidget(m_compresstype);
    typeLayout->addStretch();

    m_fileLayout = new QVBoxLayout;
    m_fileLayout->addStretch();
    m_fileLayout->addWidget(namelable);
    m_fileLayout->addWidget(m_filename);
    m_fileLayout->addStretch();
    m_fileLayout->addWidget(savetolable);
    m_fileLayout->addWidget(m_savepath);
    m_fileLayout->addWidget(m_pathbutton);
    m_fileLayout->addStretch();



    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->addStretch();
    infoLayout->addLayout(typeLayout);
    infoLayout->addSpacing(80);
    infoLayout->addLayout(m_fileLayout);
    infoLayout->addStretch();




    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addLayout(infoLayout);
    mainLayout->addSpacing(80);
    mainLayout->addWidget(m_nextbutton);
    mainLayout->addSpacing(15);
    mainLayout->addStretch();

}
void CompressSetting::InitConnection()
{
    connect(m_pathbutton, &DSuggestButton::clicked, this, &CompressSetting::onPathButoonClicked);
    connect(m_nextbutton, &DSuggestButton::clicked, this, &CompressSetting::onNextButoonClicked);
    connect(m_moresetbutton, &DSwitchButton::checkedChanged, this, &CompressSetting::onAdvanceButtonClicked);
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
        m_fileLayout->removeWidget(m_pathbutton);
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
    }
}
