#include "compresssetting.h"

#include <QDebug>
#include <DFileDialog>
#include <QFormLayout>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QMessageBox>


CompressSetting::CompressSetting(QWidget* parent)
    :QWidget(parent)
{
    m_supportedMimeTypes = m_pluginManger.supportedWriteMimeTypes(PluginManager::SortByComment);
    InitUI();
    InitConnection();
}

CompressSetting::~CompressSetting()
{

}

void CompressSetting::InitUI()
{
    m_nextbutton = new DPushButton(tr("压缩"));
    m_nextbutton->setFixedSize(340,36);


    QWidget* leftwidget = new QWidget();
    m_pixmaplabel = new DLabel();

    m_compresstype = new DComboBox();
    m_compresstype->setFixedSize(80, 40);

    for (const QString &type : qAsConst(m_supportedMimeTypes)) {
        m_compresstype->addItem(QMimeDatabase().mimeTypeForName(type).preferredSuffix());
    }
    m_compresstype->setCurrentText("zip");
    setTypeImage("zip");

    QFormLayout* filelayout = new QFormLayout();
    m_filename = new DLineEdit();
    m_filename->setFixedSize(260, 36);
    m_filename->setText(tr("新建归档文件"));
    QLineEdit* qfilename = m_filename->lineEdit();
    qfilename->setMaxLength(70);

    m_savepath = new DFileChooserEdit();
    m_savepath->setFileMode(DFileDialog::Directory);
    m_savepath->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_savepath->setFixedSize(260, 36);

    filelayout->addRow(tr("文件名") + ":", m_filename);
    filelayout->addRow(tr("保存到") + ":", m_savepath);
    filelayout->setLabelAlignment(Qt::AlignLeft);
    filelayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

    DLabel* moresetlabel = new DLabel(tr("高级选项"));
    m_moresetbutton = new DSwitchButton();
    m_moresetlayout = new QHBoxLayout();
    m_moresetlayout->addWidget(moresetlabel,0 , Qt::AlignLeft);
    m_moresetlayout->addWidget(m_moresetbutton,1 , Qt::AlignRight);


    m_encryptedlabel = new DLabel(tr("加密文件")+":");
    m_password = new DPasswordEdit();
    m_password->setFixedSize(260, 36);
    m_encryptedfilelistlabel = new DLabel(tr("加密文件列表"));
    m_file_secret = new DSwitchButton();
    m_file_secretlayout = new QHBoxLayout();
    m_file_secretlayout->addWidget(m_encryptedfilelistlabel, 0 , Qt::AlignLeft);
    m_file_secretlayout->addWidget(m_file_secret, 1 , Qt::AlignRight);
    m_splitcompress = new DLabel(tr("分卷压缩"));
    m_splitlayout = new QHBoxLayout();
    m_splitnumedit = new DDoubleSpinBox();
    m_splitnumedit->setSuffix("MB");
    m_splitnumedit->setRange(0.0, 1000000);
    m_splitnumedit->setSingleStep(0.1);
    m_splitnumedit->setDecimals(1);
    m_splitnumedit->setValue(0.0);

    m_splitlayout->addWidget(m_splitnumedit);

    m_splitlayout->setStretch(0,4);
    m_splitlayout->setStretch(1,1);
    m_splitlayout->setStretch(2,1);


    QVBoxLayout *typeLayout = new QVBoxLayout;
    typeLayout->addStretch();
    typeLayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    typeLayout->addWidget(m_compresstype, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    typeLayout->addStretch();
    leftwidget->setLayout(typeLayout);

    m_fileLayout = new QVBoxLayout;
    m_fileLayout->addStretch();
    m_fileLayout->addLayout(filelayout);
    m_fileLayout->addStretch();
    m_fileLayout->addLayout(m_moresetlayout);

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
    mainLayout->addWidget(m_nextbutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);


    m_splitnumedit->setEnabled(false);
    m_password->setEnabled(true);
    m_file_secret->setEnabled(false);
}
void CompressSetting::InitConnection()
{

    connect(m_nextbutton, &DPushButton::clicked, this, &CompressSetting::onNextButoonClicked);
    connect(m_moresetbutton, &DSwitchButton::toggled, this, &CompressSetting::onAdvanceButtonClicked);
    connect(m_compresstype, SIGNAL(currentIndexChanged(int)), this, SLOT(ontypeChanged(int)));
}




void CompressSetting::onNextButoonClicked()
{
    QString name = m_filename->text().remove(" ");
    if(name == "")
    {
        QMessageBox box;
        box.setText(tr("文件名输入错误，请重新输入！"));
        box.exec();
        return;
    }
    QMap<QString, QString> m_openArgs;
    const QString password = m_password->text();
    const QString fixedMimeType = m_supportedMimeTypes.at(m_compresstype->currentIndex());


    m_openArgs[QStringLiteral("createNewArchive")] = QStringLiteral("true");
    m_openArgs[QStringLiteral("fixedMimeType")] = fixedMimeType;
    if("application/x-tar" == fixedMimeType || "application/x-tarz" == fixedMimeType)
    {
        m_openArgs[QStringLiteral("compressionLevel")] = "-1";//-1 is unuseful
    }
    else {
        m_openArgs[QStringLiteral("compressionLevel")] = "6";//6 is default
    }

    qDebug()<<m_splitnumedit->value();
    if (m_splitnumedit->value() > 0) {
        m_openArgs[QStringLiteral("volumeSize")] = QString::number(static_cast<int>(m_splitnumedit->value()* 1024));
    }
//    if (!dialog.data()->compressionMethod().isEmpty()) {
//        m_openArgs.metaData()[QStringLiteral("compressionMethod")] = dialog.data()->compressionMethod();
//    }
    qDebug()<<m_openArgs[QStringLiteral("volumeSize")];
    if (!m_password->text().isEmpty()) {
        m_openArgs[QStringLiteral("encryptionMethod")] = "AES256";//5 is default
    }

    m_openArgs[QStringLiteral("encryptionPassword")] = password;

    if (m_file_secret->isChecked()) {
        m_openArgs[QStringLiteral("encryptHeader")] = QStringLiteral("true");
    }

    m_openArgs[QStringLiteral("localFilePath")] = m_savepath->text();
    m_openArgs[QStringLiteral("filename")] = m_filename->text() + "." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();


    emit sigCompressPressed(m_openArgs);


    m_openArgs.remove(QStringLiteral("createNewArchive"));
    m_openArgs.remove(QStringLiteral("fixedMimeType"));
    m_openArgs.remove(QStringLiteral("compressionLevel"));
    m_openArgs.remove(QStringLiteral("encryptionPassword"));
    m_openArgs.remove(QStringLiteral("encryptHeader"));
    m_openArgs.remove(QStringLiteral("localFilePath"));
    m_openArgs.remove(QStringLiteral("filename"));
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

    }
}

void CompressSetting::setTypeImage(QString type)
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

void CompressSetting::setDefaultPath(QString path)
{
    m_savepath->setText(path);
    QUrl dir(path);
    m_savepath->setDirectoryUrl(dir);
}

void CompressSetting::setDefaultName(QString name)
{
    m_filename->setText(name);
}

void CompressSetting::ontypeChanged(int index)
{
    qDebug()<<index;
    qDebug()<<m_supportedMimeTypes.at(index);

    setTypeImage(m_compresstype->itemText(index));

    if(0 == index)
    {
        m_splitnumedit->setEnabled(true);
        m_password->setEnabled(true);
        m_file_secret->setEnabled(true);
    }
    else if(10 == index)
    {
        m_splitnumedit->setEnabled(false);
        m_password->setEnabled(true);
        m_file_secret->setEnabled(false);
    }
    else {
        m_splitnumedit->setEnabled(false);
        m_password->setEnabled(false);
        m_file_secret->setEnabled(false);
    }
}

void CompressSetting::onRetrunPressed()
{

}
