/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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
#include "progress.h"

#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <QDebug>
#include <DDialog>
#include <QGraphicsDropShadowEffect>

Progress::Progress(QWidget *parent)
    : QWidget(parent)
{
    m_filename = "新建归档文件.rar";
    m_progressfile = "设计图111.jpg";
    InitUI();
    InitConnection();
}

void Progress::InitUI()
{
    m_compressicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
//    m_pixmaplabel->setPixmap(m_compressicon);
    m_filenamelabel = new DLabel();
    m_filenamelabel->setText(m_filename);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
//    font.setWeight(QFont::DemiBold);
//    m_filenamelabel->setFont(font);
    DFontSizeManager::instance()->bind(m_filenamelabel, DFontSizeManager::T5, QFont::DemiBold);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(m_filenamelabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    m_filenamelabel->setPalette(pa);

    m_progressbar = new DProgressBar();
    m_progressbar->setRange(0, 100);
    m_progressbar->setFixedSize(240, 8);
    m_progressbar->setValue(0);
    m_progressbar->setOrientation(Qt::Horizontal);  //水平方向
    m_progressbar->setAlignment(Qt::AlignVCenter);
    m_progressbar->setTextVisible(false);
    m_shadow = new DLabel();
    m_shadow->setFixedSize(8, 1);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0, 4);
    effect->setColor(QColor(44, 167, 248, 120));
    effect->setBlurRadius(6);
    m_shadow->setGraphicsEffect(effect);

    m_progressfilelabel = new DLabel();
    m_progressfilelabel->setMaximumWidth(520);
    pa = DApplicationHelper::instance()->palette(m_progressfilelabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTips));
    m_progressfilelabel->setPalette(pa);
//    m_progressfilelabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    DFontSizeManager::instance()->bind(m_progressfilelabel, DFontSizeManager::T8);
    m_progressfilelabel->setText(tr("Being calculated..."));

    m_cancelbutton = new DPushButton();
    m_cancelbutton->setFixedSize(340, 36);
    m_cancelbutton->setText(tr("Cancel"));
    m_cancelbutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_filenamelabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(25);
    mainlayout->addWidget(m_progressbar, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_shadow, 0, Qt::AlignLeft | Qt::AlignTop);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_progressfilelabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_cancelbutton, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);
}

void Progress::InitConnection()
{
    connect(m_cancelbutton, &DPushButton::clicked, this, &Progress::cancelbuttonPressedSlot);
}

int Progress::showConfirmDialog()
{
    DDialog *dialog = new DDialog;

    QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(32, 32));
    dialog->setIconPixmap(pixmap);
    DPalette pa, pa2;
    DLabel *strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    pa = DApplicationHelper::instance()->palette(strlabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::WindowText));
    strlabel->setPalette(pa);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
//    font.setWeight(QFont::Medium);
//    strlabel->setFont(font);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    DLabel *strlabel2 = new DLabel;
    strlabel2->setFixedHeight(18);
    pa2 = DApplicationHelper::instance()->palette(strlabel2);
    pa2.setBrush(DPalette::WindowText, pa2.color(DPalette::WindowText));
    strlabel2->setPalette(pa2);
//    QFont font2 = DFontSizeManager::instance()->get(DFontSizeManager::T7);
//    font2.setWeight(QFont::Medium);
//    strlabel2->setFont(font2);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T7, QFont::Medium);

    if (m_type == COMPRESSING) {
        strlabel->setText(tr("Stop compressing! "));
        strlabel2->setText(tr("There is currently a compression task in progress"));

        dialog->addButton(tr("Stop compressing"));
        dialog->addButton(tr("Continue compressing"), true, DDialog::ButtonRecommend);
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
        effect->setOffset(0, 4);
        effect->setColor(QColor(0, 145, 255, 76));
        effect->setBlurRadius(4);
        dialog->getButton(1)->setGraphicsEffect(effect);

    } else {
        strlabel->setText(tr("Stop extracting! "));
        strlabel2->setText(tr("There is currently a extraction task in progress"));
        dialog->addButton(tr("Stop extracting"));
        dialog->addButton(tr("Continue extracting"), true, DDialog::ButtonRecommend);
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
        effect->setOffset(0, 4);
        effect->setColor(QColor(0, 145, 255, 76));
        effect->setBlurRadius(4);
        dialog->getButton(1)->setGraphicsEffect(effect);
    }

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget *widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);
    return dialog->exec();
}

void Progress::cancelbuttonPressedSlot()
{
    if (0 == showConfirmDialog()) {
        emit sigCancelPressed();
    }
}

void Progress::setprogress(uint percent)
{
    m_progressbar->setValue(percent);
    m_shadow->setFixedWidth(percent * 240 / 100);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0, 4);
    effect->setColor(QColor(44, 167, 248, 120));
    effect->setBlurRadius(6);
    m_shadow->setGraphicsEffect(effect);
}

void Progress::setFilename(QString filename)
{
    QFileInfo fileinfo(filename);
    setTypeImage(fileinfo.completeSuffix());
    m_filenamelabel->setText(filename);
}

void Progress::setTypeImage(QString type)
{
    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator()  + "tempfile." + type;

    QTemporaryFile tmpFile(strTemplateName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open()) {
        tmpFile.close();
        icon = provider.icon(QFileInfo(strTemplateName));
    }

    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}

void Progress::setProgressFilename(QString filename)
{
    QFontMetrics elideFont(m_progressfilelabel->font());
    if (m_type == COMPRESSING) {
        m_progressfilelabel->setText(elideFont.elidedText(tr("Compressing") + ": " + filename, Qt::ElideMiddle, 520));
    } else {
        m_progressfilelabel->setText(elideFont.elidedText(tr("Extracting") + ": " + filename, Qt::ElideMiddle, 520));
    }
}

void Progress::settype(COMPRESS_TYPE type)
{
    m_type = type;
}
