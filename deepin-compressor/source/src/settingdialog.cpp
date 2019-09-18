#include "settingdialog.h"
#include <QStandardItemModel>
#include <qsettingbackend.h>
#include <QDebug>
#include <DFileDialog>



SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    initUI();
    initConnect();
}

void SettingDialog::initUI()
{
    const QString confDir = DStandardPaths::writableLocation(
                    QStandardPaths::AppConfigLocation);
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath);

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":/data/deepin-compressor.json");
    // 设置DSettings存储后端
    m_settings->setBackend(backend);

    // 通过DSettings对象构建设置界面
    updateSettings(m_settings);

    connect(m_settings, &DSettings::valueChanged,
            this, &SettingDialog::settingsChanged);


}

void SettingDialog::initConnect()
{

}

<<<<<<< HEAD
=======
int SettingDialog::getCurExtractPath()
{
    return m_settings->value("base.decompress.default_path").toInt();
}

bool SettingDialog::isAutoCreatDir()
{
    return m_settings->value("base.decompress.create_folder").toBool();
}

bool SettingDialog::isAutoOpen()
{
    return m_settings->value("base.decompress.open_folder").toBool();
}

>>>>>>> feat(Compressor):add settingpage
void SettingDialog::settingsChanged(const QString &key, const QVariant &value)
{
    qDebug()<<key<<value;
//    if(key == "base.decompress.default_path" && value.toInt() == 2)
//    {
//        DFileDialog dialog;
//        dialog.setAcceptMode(DFileDialog::AcceptOpen);
//        dialog.setFileMode(DFileDialog::Directory);
//        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

//        const int mode = dialog.exec();

//        if (mode != QDialog::Accepted) {
//            return;
//        }

//        QList<QUrl> pathlist = dialog.selectedUrls();

//        QString curpath = pathlist.at(0).toLocalFile();

//        auto opt = m_settings->option("base.decompress.default_path");
//        QStringList list = {"111", "222", "333"};
//        opt->setData("items", list);

//    }
    if(key.contains("file_association_type"))
    {
        QStringList associtionlist;
        associtionlist << "file_association.file_association_type.x-7z-compressed"
                       << "file_association.file_association_type.x-archive"
                       << "file_association.file_association_type.x-bcpio"
                       << "file_association.file_association_type.x-bzip"
                       << "file_association.file_association_type.x-cpio"
                       << "file_association.file_association_type.x-cpio-compressed"
                       << "file_association.file_association_type.vnd.debian.binary-package"
                       << "file_association.file_association_type.gzip"
                       << "file_association.file_association_type.x-java-archive"
                       << "file_association.file_association_type.x-lzma"
                       << "file_association.file_association_type.vnd.ms-cab-compressed"
                       << "file_association.file_association_type.vnd.rar"
                       << "file_association.file_association_type.x-rpm"
                       << "file_association.file_association_type.x-sv4cpio"
                       << "file_association.file_association_type.x-sv4crc"
                       << "file_association.file_association_type.x-tar"
                       << "file_association.file_association_type.x-bzip-compressed-tar"
                       << "file_association.file_association_type.x-compressed-tar"
                       << "file_association.file_association_type.x-lzip-compressed-tar"
                       << "file_association.file_association_type.x-lzma-compressed-tar"
                       << "file_association.file_association_type.x-tzo"
                       << "file_association.file_association_type.x-xz-compressed-tar"
                       << "file_association.file_association_type.x-tarz"
                       << "file_association.file_association_type.x-xar"
                       << "file_association.file_association_type.x-xz"
                       << "file_association.file_association_type.zip"
                       << "file_association.file_association_type.x-cd-image"
                       << "file_association.file_association_type.x-iso9660-appimage"
                       << "file_association.file_association_type.x-source-rpm";
        foreach(QString key, associtionlist)
        {
            bool value = m_settings->value(key).toBool();
            QString mimetype = "application/" + key.remove("file_association.file_association_type.");
            startcmd(mimetype, value);
        }


    }

}

void SettingDialog::startcmd(QString &mimetype, bool state)
{
    if(!m_process)
    {
        m_process = new KProcess;
    }

    QString programPath = QStandardPaths::findExecutable("xdg-mime");
    if (programPath.isEmpty()) {
        qDebug()<<"error can't find xdg-mime";
        return;
    }

    QStringList arguments;

    arguments<<"default"<<"deepin-compressor.desktop"<<mimetype;

    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    m_process->setProgram(programPath, arguments);
    m_process->start();
    m_process->waitForStarted();
    m_process->waitForFinished();
}
