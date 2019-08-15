/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "uncompresspage.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTextLayout>
#include <QSvgWidget>
#include <QFileInfo>
#include <QUrlQuery>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QDir>



UnCompressPage::UnCompressPage(QWidget *parent)
    : QWidget(parent),
      m_nameLabel(new QLabel),
      m_styleLabel(new QLabel),
      m_typeLabel(new QLabel),
      m_versionLabel(new QLabel),
      m_copyrightLabel(new QLabel),
      m_descriptionLabel(new QLabel),
      m_tipsLabel(new QLabel()),
      m_installBtn(new DSuggestButton),
      m_uninstallBtn(new QPushButton(tr("Remove"))),
      m_reinstallBtn(new QPushButton(tr("Reinstall"))),
      m_viewFileBtn(new DSuggestButton),
      m_closeBtn(new DSuggestButton),
      m_spinner(new DSpinner),
      m_bottomLayout(new QStackedLayout)
{
    QSvgWidget *iconWidget = new QSvgWidget(":/images/font-x-generic.svg");
    iconWidget->setFixedSize(70, 70);

    QLabel *styleLabel = new QLabel(tr("Style: "));
    QLabel *typeLabel = new QLabel(tr("Type: "));
    QLabel *versionLabel = new QLabel(tr("Version: "));
    QLabel *copyrightLabel = new QLabel(tr("Copyright: "));
    QLabel *descLabel = new QLabel(tr("Description: "));

    copyrightLabel->setAlignment(Qt::AlignTop);
    descLabel->setAlignment(Qt::AlignTop);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(styleLabel, m_styleLabel);
    formLayout->addRow(typeLabel, m_typeLabel);
    formLayout->addRow(versionLabel, m_versionLabel);
    formLayout->addRow(copyrightLabel, m_copyrightLabel);
    formLayout->addRow(descLabel, m_descriptionLabel);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setHorizontalSpacing(5);
    formLayout->setVerticalSpacing(7);

    QWidget *btnsWidget = new QWidget;
    QHBoxLayout *btnsLayout = new QHBoxLayout(btnsWidget);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installBtn);
    btnsLayout->addWidget(m_uninstallBtn);
    btnsLayout->addWidget(m_reinstallBtn);
    btnsLayout->addWidget(m_viewFileBtn);
    btnsLayout->addWidget(m_closeBtn);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(20);
    btnsLayout->setMargin(0);

    QWidget *progressWidget = new QWidget;
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    progressWidget->setFixedHeight(40);
    m_spinner->setFixedSize(30, 30);

    m_bottomLayout->addWidget(btnsWidget);
    m_bottomLayout->addWidget(progressWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(iconWidget, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(m_bottomLayout);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(50, 0, 50, 20);

    // init property.
    styleLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    typeLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    versionLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    copyrightLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    descLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");

    m_nameLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: 510; }");
    m_styleLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_typeLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_versionLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_copyrightLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_descriptionLabel->setStyleSheet("QLabel { font-size: 14px; }");

    m_copyrightLabel->setAlignment(Qt::AlignTop);
    m_copyrightLabel->setWordWrap(true);

    m_descriptionLabel->setAlignment(Qt::AlignTop);
    m_descriptionLabel->setWordWrap(true);
    m_tipsLabel->setText("");

    m_installBtn->setText(tr("Install"));
    m_installBtn->setFixedSize(160, 36);
    m_installBtn->setFocusPolicy(Qt::NoFocus);
    m_installBtn->setVisible(false);

    m_uninstallBtn->setFixedSize(160, 36);
    m_uninstallBtn->setFocusPolicy(Qt::NoFocus);
    m_uninstallBtn->setObjectName("GrayButton");
    m_uninstallBtn->setVisible(false);

    m_reinstallBtn->setFixedSize(160, 36);
    m_reinstallBtn->setFocusPolicy(Qt::NoFocus);
    m_reinstallBtn->setObjectName("GrayButton");
    m_reinstallBtn->setVisible(false);

    m_viewFileBtn->setText(tr("View font directory"));
    m_viewFileBtn->setFixedSize(180, 36);
    m_viewFileBtn->setFocusPolicy(Qt::NoFocus);
    m_viewFileBtn->setVisible(false);

    m_closeBtn->setFixedSize(160, 36);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setVisible(false);
    m_closeBtn->setText(tr("Done"));

    // connect the signals to the slots function.

    connect(m_installBtn, &QPushButton::clicked, this, &UnCompressPage::handleInstall);
    connect(m_uninstallBtn, &QPushButton::clicked, this, &UnCompressPage::handleRemove);
    connect(m_reinstallBtn, &QPushButton::clicked, this, &UnCompressPage::handleReinstall);
    connect(m_viewFileBtn, &QPushButton::clicked, this, &UnCompressPage::viewFilePath);
    connect(m_closeBtn, &QPushButton::clicked, this, &QApplication::quit);

}

UnCompressPage::~UnCompressPage()
{
}

void UnCompressPage::updateInfo()
{
    refreshPage();


}

void UnCompressPage::refreshPage()
{

        m_tipsLabel->setText("");
        m_installBtn->setVisible(true);
        m_uninstallBtn->setVisible(false);
        m_reinstallBtn->setVisible(false);
        m_viewFileBtn->setVisible(false);
        m_closeBtn->setVisible(false);
}

void UnCompressPage::progressBarStart()
{
    m_bottomLayout->setCurrentIndex(1);
    m_tipsLabel->setText("");
    m_spinner->start();
}

void UnCompressPage::handleInstall()
{

}

void UnCompressPage::handleRemove()
{

}

void UnCompressPage::handleReinstall()
{

}

void UnCompressPage::onInstallFinished()
{

}

void UnCompressPage::onUninstallFinished()
{

}

void UnCompressPage::onReinstallFinished()
{

}

void UnCompressPage::viewFilePath()
{
    QUrl url = QUrl::fromLocalFile(QFileInfo(m_filePath).dir().absolutePath());
    QUrlQuery query;
    query.addQueryItem("selectUrl", QUrl::fromLocalFile(m_filePath).toString());
    url.setQuery(query);

    QProcess::startDetached("dde-file-manager", QStringList(url.toString()));
}
