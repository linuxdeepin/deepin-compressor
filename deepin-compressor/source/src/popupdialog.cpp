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
#include "utils.h"
#include "queries.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DCheckBox>
#include <DRadioButton>
#include <QEvent>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QDebug>
#include <DPasswordEdit>

TipDialog::TipDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
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

TipDialog::NewStr TipDialog::autoCutText(const QString &text, DLabel *pDesLbl)
{
    if (text.isEmpty() || nullptr == pDesLbl) {
        return NewStr();
    }

    QFont font; // 应用使用字体对象
    QFontMetrics font_label(font);
    QString strText = text;
    int titlewidth = font_label.width(strText);
    QString str;
    NewStr newstr;
    int width = pDesLbl->width();
    if (titlewidth < width) {
        newstr.strList.append(strText);
        newstr.resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (font_label.width(str) > width) { //根据label宽度调整每行字符数
                str.remove(str.count() - 1, 1);
                newstr.strList.append(str);
                newstr.resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        newstr.strList.append(str);
        newstr.resultStr += str;
    }
    newstr.fontHeifht = font_label.height();
    return newstr;
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
