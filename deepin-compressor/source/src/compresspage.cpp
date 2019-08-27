

#include "compresspage.h"
#include "utils.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <DRecentManager>



DWIDGET_USE_NAMESPACE

CompressPage::CompressPage(QWidget *parent)
    : QWidget(parent)
{
    m_fileviewer = new FileViewer();
    m_model = new QFileSystemModel();
    m_model->setRootPath(QDir::currentPath());
    m_nextbutton = new DSuggestButton(tr("NEXT"));
    m_nextbutton->setFixedWidth(260);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_nextbutton);
    buttonlayout->addStretch();


    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 10);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(0, 0, 0, 20);

    m_settings = new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                             QSettings::IniFormat);
    // initalize the configuration file.
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    setupView();

    connect(m_nextbutton, &DSuggestButton::clicked, this, &CompressPage::onNextPress);
}

CompressPage::~CompressPage()
{

}

void CompressPage::setupView()
{



    m_fileviewer->setModel(m_model);
    m_fileviewer->setRootIndex(m_model->index(QDir::currentPath()));



}

void CompressPage::addItems(const QStringList &paths)
{


}

void CompressPage::onNextPress()
{
    emit sigNextPress();
}

void CompressPage::onAddfileSlot()
{
    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);

    QString historyDir = m_settings->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }
}





