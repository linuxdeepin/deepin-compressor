#include "progressdialog.h"

#include <QBoxLayout>
#include <QFileIconProvider>
#include <QDebug>

ProgressDialog::ProgressDialog(QWidget *parent):
    QDialog(parent)
{
    initUI();
    initConnect();
}

void ProgressDialog::initUI()
{
//    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setFixedWidth(m_defaultWidth);
    setWindowTitle(QObject::tr("提取中..."));

    m_tasklable = new DLabel();
    m_filelable = new DLabel();
    m_tasklable->setText(tr("当前任务") + ":");
    m_filelable->setText(tr("正在提取") + ":");

    m_circleprogress = new  DCircleProgress();
    m_circleprogress->setFixedSize(80, 80);
    m_circleprogress->setBackgroundColor(QColor(238, 238, 238));
    m_circleprogress->setChunkColor(QColor(90, 90, 255));
    m_circleprogress->setLineWidth(5);
    m_circleprogress->setValue(0);
    m_circleprogress->setText("0");


    QVBoxLayout* rightlaout = new QVBoxLayout;
    rightlaout->addWidget(m_tasklable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    rightlaout->addWidget(m_filelable, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QHBoxLayout* mainlayout = new QHBoxLayout;
    mainlayout->addWidget(m_circleprogress, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainlayout->addLayout(rightlaout, 0);

    mainlayout->setStretch(0,1);
    mainlayout->setStretch(1,4);

    setLayout(mainlayout);

    m_extractdialog = new ExtractPauseDialog();
}

void ProgressDialog::initConnect()
{
    connect(m_extractdialog, &ExtractPauseDialog::sigbuttonpress, this, &ProgressDialog::slotextractpress);
}

void ProgressDialog::slotextractpress(int index)
{
    qDebug()<<index;
    if(0 == index)
    {
        emit stopExtract();
    }
    else {
        exec();
    }
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    if(m_circleprogress->value() < 100 && m_circleprogress->value() > 0)
    {
        accept();
        m_extractdialog->exec();
    }

}

void ProgressDialog::setCurrentTask(const QString &file)
{
    QFileIconProvider icon_provider;
    QFileInfo fileinfo(file);
    QIcon icon = icon_provider.icon(fileinfo);
//    setIcon(icon, QSize(16, 16));
    setWindowIcon(icon);

    m_tasklable->setText(tr("当前任务") + ":" + fileinfo.fileName());
}

void ProgressDialog::setCurrentFile(const QString &file)
{
    QFileInfo fileinfo(file);
    m_filelable->setText(tr("正在提取") + ":" + fileinfo.fileName());
}

void ProgressDialog::setProcess(unsigned long  value)
{
    if(value > m_circleprogress->value())
    {
        m_circleprogress->setValue(value);
        m_circleprogress->setText(QString::number(value));
    }
}

void ProgressDialog::setFinished(const QString &path)
{
    setWindowTitle(tr(""));
    m_circleprogress->setValue(100);
    m_circleprogress->setText(QString::number(100));
    m_filelable->setText(tr("提取完成") + ":" + tr("已提取到") + path);
    m_extractdialog->reject();
    reject();
}

void ProgressDialog::showdialog()
{
    exec();
}

bool ProgressDialog::isshown()
{
    return this->isVisible() || m_extractdialog->isVisible();
}

