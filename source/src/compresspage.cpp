

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
    m_nextbutton->setFixedWidth(260);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addStretch();
    contentLayout->addWidget(m_fileviewer);
    contentLayout->addStretch();

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_nextbutton);
    buttonlayout->addStretch();


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonlayout);
    mainLayout->addStretch();

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






