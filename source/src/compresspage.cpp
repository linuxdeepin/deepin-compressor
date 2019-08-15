

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
    m_fileviewer = new fileViewer();
    m_nextbutton = new DSuggestButton(tr("NEXT"));

    QBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addSpacing(15);
    contentLayout->addWidget(m_fileviewer);
    contentLayout->addSpacing(15);


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(contentLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_nextbutton);
    mainLayout->addSpacing(15);

    connect(m_nextbutton, &DSuggestButton::clicked, this, &CompressPage::onNextPress);
}

CompressPage::~CompressPage()
{

}

void CompressPage::addItems(const QStringList &paths)
{


}

void CompressPage::onNextPress()
{
    emit sigNextPress();
}






