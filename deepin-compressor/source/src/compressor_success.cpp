/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#include "compressor_success.h"
#include "customwidget.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DFileDialog>
#include <DDesktopServices>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QDebug>

#include <utils.h>

Compressor_Success::Compressor_Success(QWidget *parent)
    : DWidget(parent)
{
    m_stringinfo = tr("Compression successful");
    InitUI();
    InitConnection();
}

void Compressor_Success::InitUI()
{
    m_compressicon = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_128px.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_pixmaplabel->setPixmap(m_compressicon);

    m_stringinfolabel = new DLabel(this);

    DFontSizeManager::instance()->bind(m_stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    m_stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    m_stringinfolabel->setText(m_stringinfo);
    m_showfilebutton = new CustomPushButton(this);
    m_showfilebutton->setMinimumSize(340, 36);
    m_showfilebutton->setText(tr("View"));

    commandLinkBackButton = new CustomCommandLinkButton(tr("Back"), this);
    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(commandLinkBackButton);
    commandLinkButtonLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_showfilebutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);

    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
}

void Compressor_Success::InitConnection()
{
    connect(m_showfilebutton, &DPushButton::clicked, this, &Compressor_Success::showfiledirSlot);
    connect(commandLinkBackButton, &DCommandLinkButton::clicked, this, &Compressor_Success::commandLinkBackButtonClicked);

//    auto changeTheme = [this]() {
//        DPalette pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
//        pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
//        m_stringinfolabel->setPalette(pa);
//    };

//    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);
}

void Compressor_Success::showfiledirSlot(bool iIsUrl)
{
    if (m_convertType.size() == 0) {
        if (newCreatePath_.isEmpty() == false) {
            if (iIsUrl) {
                DDesktopServices::showFolder(QUrl(newCreatePath_, QUrl::TolerantMode));
            } else {
                DDesktopServices::showFolder(newCreatePath_);
            }
        } else if (m_fullpath.isEmpty()) {
            if (iIsUrl) {
                DDesktopServices::showFolder(QUrl(m_path, QUrl::TolerantMode));
            } else {
                DDesktopServices::showFolder(m_path);
            }
        } else {
            QFileInfo fileInfo(m_fullpath);
            if (fileInfo.isDir()) {
                if (iIsUrl) {
                    DDesktopServices::showFolder(QUrl(m_fullpath, QUrl::TolerantMode));
                } else {
                    DDesktopServices::showFolder(m_fullpath);
                }
            } else if (fileInfo.isFile()) {
                if (iIsUrl) {
                    DDesktopServices::showFileItem(QUrl(m_fullpath, QUrl::TolerantMode));
                } else {
                    DDesktopServices::showFileItem(m_fullpath);
                }
            }
        }
    } else {
        emit sigOpenConvertArchive(m_fullpath);
    }
}

void Compressor_Success::commandLinkBackButtonClicked()
{
    emit sigBackButtonClicked();
}

void Compressor_Success::setstringinfo(QString str)
{
    m_stringinfolabel->setText(str);
}

void Compressor_Success::setCompressPath(QString path)
{
    qDebug() << path;
    m_path = path;
}

QString Compressor_Success::getPath()
{
    return m_path;
}

void Compressor_Success::setCompressFullPath(const QString &path)
{
    qDebug() << path;
    m_fullpath = path;
}

void Compressor_Success::setCompressNewFullPath(const QString &path)
{
    newCreatePath_ = path;
}

void Compressor_Success::clear()
{
    setCompressFullPath("");
    setCompressNewFullPath("");
}

void Compressor_Success::setConvertType(QString type)
{
    m_convertType = type;
}

CustomPushButton *Compressor_Success::getShowfilebutton()
{
    return m_showfilebutton;
}

//bool Compressor_Success::eventFilter(QObject *watched, QEvent *event)
//{
//    if (watched == m_showfilebutton) {
//        if (QEvent::KeyPress == event->type()) {
//            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//            if (Qt::Key_Enter == keyEvent->key() || Qt::Key_Return == keyEvent->key()) {
//                QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
//                QApplication::sendEvent(m_showfilebutton, &pressSpace);
//                // 设置定时
//                QTimer::singleShot(80, this, [&]() {
//                    // 模拟空格键松开事件
//                    QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
//                    QApplication::sendEvent(m_showfilebutton, &releaseSpace);
//                });
//                return true;
//            } else {
//                return false;
//            }
//        }
//    } else {
//        return DWidget::eventFilter(watched, event);
//    }
//}
