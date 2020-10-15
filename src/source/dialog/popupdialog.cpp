/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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

#include "popupdialog.h"
#include "uitools.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DCheckBox>

#include <QVBoxLayout>
#include <QDebug>

TipDialog::TipDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(380, 140);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

TipDialog::~TipDialog()
{

}

int TipDialog::showDialog(const QString &strDesText, const QString btnMsg, ButtonType btnType)
{
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setMinimumSize(293, 20);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setWordWrap(true);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg, true, btnType);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addWidget(pDesLbl, 0, Qt::AlignVCenter);
    pMainLayout->addSpacing(10);

    DWidget *widget = new DWidget(this);

    widget->setLayout(pMainLayout);
    addContent(widget);

    return exec();
}

SimpleQueryDialog::SimpleQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(380, 140);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

SimpleQueryDialog::~SimpleQueryDialog()
{

}

int SimpleQueryDialog::showDialog(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setMinimumSize(293, 20);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setWordWrap(true);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg1, true, btnType1);
    addButton(btnMsg2, true, btnType2);
    if (btnMsg3.size() > 0) {
        addButton(btnMsg3, true, btnType3);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addWidget(pDesLbl, 0, Qt::AlignVCenter);
    pMainLayout->addSpacing(10);

    DWidget *widget = new DWidget(this);

    widget->setLayout(pMainLayout);
    addContent(widget);

    return exec();
}

OverwriteQueryDialog::OverwriteQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(380, 190);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

OverwriteQueryDialog::~OverwriteQueryDialog()
{

}

void OverwriteQueryDialog::showDialog(QString file)
{
    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(280, 20));
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    int limitCounts = 16;
    int left = 8, right = 8;
    QString fileName = file;
    QString displayName = "";
    displayName = fileName.length() > limitCounts ? fileName.left(left) + "..." + fileName.right(right) : fileName;
    strlabel->setText(displayName);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(154, 20));
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setForegroundRole(DPalette::ToolTipText);

    strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));

    addButton(QObject::tr("Skip"));
    addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);

    DCheckBox *checkbox = new DCheckBox;
    checkbox->setAccessibleName("Applyall_btn");
    checkbox->setStyleSheet("QCheckBox::indicator {width: 21px; height: 21px;}");

    DLabel *checkLabel = new DLabel(QObject::tr("Apply to all"));
    checkLabel->setMinimumSize(QSize(98, 20));
    DFontSizeManager::instance()->bind(checkLabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *checkLayout = new QHBoxLayout;
    checkLayout->addStretch();
    checkLayout->addWidget(checkbox);
    checkLayout->addWidget(checkLabel);
    checkLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addLayout(checkLayout);

    DWidget *widget = new DWidget(this);
    widget->setLayout(mainlayout);
    addContent(widget);

    const int mode = exec();
    m_ret = mode;
    m_applyAll = checkbox->isChecked();

    if (-1 == mode) {
        m_retType = OR_Cancel;
    } else if (0 == mode) {
        if (m_applyAll) {
            m_retType = OR_SkipAll;
        } else {
            m_retType = OR_Skip;
        }
    } else if (1 == mode) {
        if (m_applyAll) {
            m_retType = OR_OverwriteAll;
        } else {
            m_retType = OR_Overwrite;
        }
    }

    qDebug() << "mode" << mode;
    qDebug() << "applyall" << m_applyAll;
    qDebug() << "retType" << m_retType;
}

int OverwriteQueryDialog::getDialogResult()
{
    return m_ret;
}

int OverwriteQueryDialog::getQueryResult()
{
    return m_retType;
}

bool OverwriteQueryDialog::getApplyAll()
{
    return m_applyAll;
}
