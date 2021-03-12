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
#include "queries.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DCheckBox>
#include <DRadioButton>
#include <QEvent>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QDebug>

TipDialog::TipDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

TipDialog::~TipDialog()
{

}

int TipDialog::showDialog(const QString &strDesText, const QString btnMsg, ButtonType btnType, const QString &strToolTip)
{
    m_strDesText = strDesText;
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setObjectName("ContentLabel");
    pDesLbl->setFixedWidth(340);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setAlignment(Qt::AlignCenter);
    // 设置提示信息
    if (!strToolTip.isEmpty())
        pDesLbl->setToolTip(strToolTip);

    // 确定按钮
    addButton(btnMsg, true, btnType);
    addContent(pDesLbl, Qt::AlignHCenter); // 使用Qt::AlignHCenter效果最好

    autoFeed(pDesLbl);

    return exec();
}

void TipDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iDialogOldHeight = height();
}

void TipDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }
    DDialog::changeEvent(event);
}
SimpleQueryDialog::SimpleQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

SimpleQueryDialog::~SimpleQueryDialog()
{

}

int SimpleQueryDialog::showDialog(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    m_strDesText = strDesText;
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setObjectName("ContentLabel");
    pDesLbl->setFixedWidth(340);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg1, true, btnType1);
    addButton(btnMsg2, true, btnType2);
    if (btnMsg3.size() > 0) {
        addButton(btnMsg3, true, btnType3);
    }

    addContent(pDesLbl, Qt::AlignHCenter);

    autoFeed(pDesLbl);

    return exec();
}

void SimpleQueryDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iDialogOldHeight = height();
}

void SimpleQueryDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }

    DDialog::changeEvent(event);
}

OverwriteQueryDialog::OverwriteQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

OverwriteQueryDialog::~OverwriteQueryDialog()
{

}

void OverwriteQueryDialog::showDialog(QString file, bool bDir)
{
    m_strFilesname = file;
    DLabel *strlabel = new DLabel;
    strlabel->setObjectName("NameLabel");
    strlabel->setFixedWidth(340);
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setObjectName("ContentLabel");
    strlabel2->setFixedWidth(340);
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setForegroundRole(DPalette::ToolTipText);

    if (bDir) {
        // 文件夹提示
        strlabel2->setText(QObject::tr("Another folder with the same name already exists, replace it?"));
        m_strDesText = strlabel2->text();
        addButton(QObject::tr("Skip"));
        addButton(QObject::tr("Merge"), true, DDialog::ButtonWarning);
    } else {
        // 文件提示
        strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));
        m_strDesText = strlabel2->text();
        addButton(QObject::tr("Skip"));
        addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);
    }

    DCheckBox *checkbox = new DCheckBox(tr("Apply to all"));
    DFontSizeManager::instance()->bind(checkbox, DFontSizeManager::T6, QFont::Medium);
    checkbox->setAccessibleName("Applyall_btn");

    addContent(strlabel2, Qt::AlignHCenter);
    addContent(strlabel, Qt::AlignHCenter);
    addContent(checkbox, Qt::AlignHCenter);

    //setTabOrder需放在布局最后，否则不生效
    this->setTabOrder(checkbox, this->getButton(0));
    this->setTabOrder(this->getButton(0), this->getButton(1));

    autoFeed(strlabel, strlabel2);

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

void OverwriteQueryDialog::autoFeed(DLabel *label1, DLabel *label2)
{
    NewStr newstr = autoCutText(m_strDesText, label2);
    label2->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label2->setFixedHeight(height_lable);

    // 字符串太长的情况下用中间使用...
    QFont font;
    QFontMetrics elideFont(font);
    label1->setText(elideFont.elidedText(m_strFilesname, Qt::ElideMiddle, 340));

    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height - m_iCheckboxOld1Height + height_lable + 2 * newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iCheckboxOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = height();
}

void OverwriteQueryDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            Dtk::Widget::DLabel *pNameLabel = findChild<Dtk::Widget::DLabel *>("NameLabel");
            if (nullptr != pNameLabel) {
                autoFeed(pNameLabel, p);
            }
        }
    }

    DDialog::changeEvent(event);
}


ConvertDialog::ConvertDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380); // 提示框宽度固定

    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

ConvertDialog::~ConvertDialog()
{

}

QStringList ConvertDialog::showDialog()
{
    DLabel *strlabel = new DLabel;
    strlabel->setObjectName("ContentLabel");
    strlabel->setFixedWidth(340); // 宽度固定
    strlabel->setText(tr("Changes to archives in this file type are not supported. Please convert the archive format to save the changes."));
    m_strDesText = strlabel->text();
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(112, 20));
    strlabel2->setText(tr("Convert the format to:"));
    strlabel2->setAlignment(Qt::AlignCenter);
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
    labelLayout->setContentsMargins(0, 0, 0, 0);

    DWidget *widget = new DWidget();
    widget->setLayout(labelLayout);
    addContent(strlabel, Qt::AlignHCenter); // 使用Qt::AlignHCenter效果最好
    addContent(widget, Qt::AlignHCenter);
    // 添加取消和转换按钮
    addButton(tr("Cancel"));
    addButton(tr("Convert"), true, DDialog::ButtonRecommend);

    // 设置焦点顺序
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
        qInfo() << "zip" << isZipConvert;
    });

    // 转换为7z格式
    connect(_7zBtn, &DRadioButton::toggled, this, [ =, &is7zConvert]() {
        is7zConvert = _7zBtn->isChecked();
        qInfo() << "7z" << is7zConvert;
    });

    autoFeed(strlabel);

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

void ConvertDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height + height_lable + newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = height();
}

void ConvertDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }

    DDialog::changeEvent(event);
}
