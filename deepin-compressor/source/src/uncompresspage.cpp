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








}

UnCompressPage::~UnCompressPage()
{
}


