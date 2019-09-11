#include "progressdialog.h"

#include <QBoxLayout>
#include <QFileIconProvider>

ProgressDialog::ProgressDialog(QWidget *parent):
    DDialog(parent)
{
    initUI();
    initConnect();
}

void ProgressDialog::initUI()
{
    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window);
    setFixedWidth(m_defaultWidth);
    setTitle(tr("提取中..."));

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

    insertContent(0, m_circleprogress, Qt::AlignLeft | Qt::AlignVCenter);
    insertContent(1, m_tasklable);
    addSpacing(10);
    insertContent(2, m_filelable);

    moveToCenter();
}

void ProgressDialog::initConnect()
{

}


void ProgressDialog::setCurrentTask(const QString &file)
{
    QFileIconProvider icon_provider;
    QFileInfo fileinfo(file);
    QIcon icon = icon_provider.icon(fileinfo);
    setIcon(icon, QSize(16, 16));

    m_tasklable->setText(tr("当前任务") + ":" + fileinfo.fileName());
}

void ProgressDialog::setCurrentFile(const QString &file)
{
    QFileInfo fileinfo(file);
    m_filelable->setText(tr("正在提取") + ":" + fileinfo.fileName());
}

void ProgressDialog::setProcess(unsigned long  value)
{
    m_circleprogress->setValue(value);
    m_circleprogress->setText(QString::number(value));
}

void ProgressDialog::setFinished(const QString &path)
{
    setTitle(tr(""));
    m_circleprogress->setValue(100);
    m_circleprogress->setText(QString::number(100));
    m_filelable->setText(tr("提取完成") + ":" + "已提取到" + path);
}
