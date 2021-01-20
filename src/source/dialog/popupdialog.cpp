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
#include <DRadioButton>

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
    pMainLayout->setContentsMargins(10, 0, 10, 0);
    pMainLayout->addWidget(pDesLbl, 0, Qt::AlignVCenter);
//    pMainLayout->addSpacing(10);

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
    pMainLayout->setContentsMargins(10, 0, 10, 0);
    pMainLayout->addWidget(pDesLbl, 1, Qt::AlignVCenter);
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

void OverwriteQueryDialog::showDialog(QString file, bool bDir)
{
    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(280, 20));
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    // 字符串太长的情况下用中间使用...
    QFontMetrics elideFont(strlabel->font());
    strlabel->setText(elideFont.elidedText(file, Qt::ElideMiddle, 280));

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(154, 20));
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setForegroundRole(DPalette::ToolTipText);
    strlabel2->setWordWrap(true);

    if (bDir) {
        // 文件夹提示
        strlabel2->setText(QObject::tr("Another folder with the same name already exists, replace it?"));
        addButton(QObject::tr("Skip"));
        addButton(QObject::tr("Merge"), true, DDialog::ButtonWarning);
    } else {
        // 文件提示
        strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));
        addButton(QObject::tr("Skip"));
        addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);
    }

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
    mainlayout->setContentsMargins(10, 0, 10, 0);
    mainlayout->addWidget(strlabel2, 1);
    mainlayout->addWidget(strlabel, 0);
    mainlayout->addLayout(checkLayout);

    DWidget *widget = new DWidget(this);
    widget->setLayout(mainlayout);
    addContent(widget);

    //setTabOrder需放在布局最后，否则不生效
    this->setTabOrder(checkbox, this->getButton(0));
    this->setTabOrder(this->getButton(0), this->getButton(1));

    const int mode = exec();
    m_ret = mode;
    m_applyAll = checkbox->isChecked();  // 是否应用到全部文件

    if (-1 == mode) {
        m_retType = OR_Cancel;   // 取消
    } else if (0 == mode) {
        if (m_applyAll) {
            m_retType = OR_SkipAll;   // 全部跳过
        } else {
            m_retType = OR_Skip;  // 跳过
        }
    } else if (1 == mode) {
        if (m_applyAll) {
            m_retType = OR_OverwriteAll;  // 全部替换
        } else {
            m_retType = OR_Overwrite;  // 替换
        }
    }
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

ConvertDialog::ConvertDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(QSize(380, 180));

    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

ConvertDialog::~ConvertDialog()
{

}

QStringList ConvertDialog::showDialog()
{
    // 添加取消和转换按钮
    addButton(tr("Cancel"));
    addButton(tr("Convert"), true, DDialog::ButtonRecommend);

    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(308, 40));
    strlabel->setText(tr("Changes to archives in this file type are not supported. Please convert the archive format to save the changes."));
    strlabel->setWordWrap(true);
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *textLayout = new QHBoxLayout;
    textLayout->setSpacing(36);
    textLayout->addWidget(strlabel/*, Qt::AlignHCenter | Qt::AlignVCenter*/);
    textLayout->setSpacing(36);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(112, 20));
    strlabel2->setText(tr("Convert the format to:"));
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);

    DRadioButton *zipBtn = new DRadioButton("ZIP");
    zipBtn->setChecked(true);
    DRadioButton *_7zBtn = new DRadioButton("7Z");
    _7zBtn->setChecked(false);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addStretch();
    labelLayout->addWidget(strlabel2);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(zipBtn);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(_7zBtn);
    labelLayout->addStretch();

    DWidget *widget = new DWidget(this);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addWidget(strlabel);
    mainlayout->addStretch();
    mainlayout->addLayout(labelLayout);

    widget->setLayout(mainlayout);

    addContent(widget);
    setTabOrder(zipBtn, _7zBtn);
    setTabOrder(_7zBtn, getButton(0));
    setTabOrder(getButton(0), getButton(1));

    QStringList typeList;
    typeList << "false" << "";
    bool isZipConvert = true;
    bool is7zConvert = false;
    QString convertType;

    // 转换zip格式
    connect(zipBtn, &DRadioButton::toggled, this, [ =, &isZipConvert]() {
        isZipConvert = zipBtn->isChecked();
        qDebug() << "zip" << isZipConvert;
    });

    // 转换为7z格式
    connect(_7zBtn, &DRadioButton::toggled, this, [ =, &is7zConvert]() {
        is7zConvert = _7zBtn->isChecked();
        qDebug() << "7z" << is7zConvert;
    });

    const int mode = exec();

    if (mode == QDialog::Accepted) {
        typeList.clear();
        if (isZipConvert) {
            typeList << "true" << "zip";
        } else if (is7zConvert) {
            typeList << "true" << "7z";
        }
    } else {
        typeList << "false" << "none";
    }

    return typeList;
}
