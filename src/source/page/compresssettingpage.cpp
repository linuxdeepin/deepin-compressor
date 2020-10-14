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

#include "compresssettingpage.h"
#include "customwidget.h"
#include "pluginmanager.h"
#include "uitools.h"
#include "uistruct.h"
#include "popupdialog.h"

#include <DApplication>
#include <DFileDialog>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QStandardPaths>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFormLayout>
#include <QFileIconProvider>
#include <QMenu>
#include <QMimeDatabase>
#include <QDebug>

TypeLabel::TypeLabel(QWidget *parent)
    : DLabel(parent)
{

}

TypeLabel::~TypeLabel()
{

}

void TypeLabel::mousePressEvent(QMouseEvent *event)
{
    emit labelClickEvent(event);
    DLabel::mousePressEvent(event);
}

void TypeLabel::paintEvent(QPaintEvent *event)
{
    if (hasFocus() && (m_reson & (Qt::TabFocusReason | Qt::BacktabFocusReason | Qt::PopupFocusReason))) {
        DStylePainter painter(this);
        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        const QStyleOptionFrame *opt1 = &opt;
        DStyleHelper dstyle(style);
        int border_width = dstyle.pixelMetric(DStyle::PM_FocusBorderWidth, opt1, this);
        QColor color = dstyle.getColor(opt1, QPalette::Highlight);
        painter.setPen(QPen(color, border_width, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, opt1, &painter, this);
    }
    QLabel::paintEvent(event);
}

void TypeLabel::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    QLabel::focusInEvent(event);
}

void TypeLabel::focusOutEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    QLabel::focusOutEvent(event);
}



CompressSettingPage::CompressSettingPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

CompressSettingPage::~CompressSettingPage()
{

}

void CompressSettingPage::setFileSize(const QStringList &listFiles, qint64 qSize)
{
    m_listFiles = listFiles;
    m_qFileSize = qSize;
}

void CompressSettingPage::setCompressSavePath(const QString &strPath)
{
    m_pSavePathEdt->setText(strPath);
    QUrl dir(strPath);
    m_pSavePathEdt->setDirectoryUrl(dir);
}

void CompressSettingPage::refreshMenu()
{
    if (m_listSupportedMimeTypes.empty()) {
        m_listSupportedMimeTypes = PluginManager::get_instance().supportedWriteMimeTypes(PluginManager::SortByComment);     // 获取支持的压缩格式
    }

    QAction *pAction = nullptr;
    bool bHas7z = false;
    for (const QString &type : qAsConst(m_listSupportedMimeTypes)) {
        if (QMimeDatabase().mimeTypeForName(type).preferredSuffix() == "7z") {
            bHas7z = true;
        }
        pAction = new QAction(QMimeDatabase().mimeTypeForName(type).preferredSuffix());
        pAction->setData(type);
        m_pTypeMenu->addAction(pAction);

    }

    // 判断是否安装了7z，若安装了7z，则压缩类型添加tar.7z
    if (bHas7z) {
        // 判断是否已经有压缩类型，tzr.7z添加在zip前面
        if (pAction != nullptr) {
            m_pTypeMenu->insertAction(pAction, new QAction("tar.7z"));
        } else {
            m_pTypeMenu->addAction("tar.7z");
        }
    }

    // 默认选择类型为zip
    if (pAction != nullptr) {
        slotTypeChanged(pAction);
    }

}

void CompressSettingPage::initUI()
{
    // 左侧界面
    m_pTypePixmapLbl = new DLabel(this);
    m_pClickLbl = new TypeLabel(this);
    m_pCompressTypeLbl = new TypeLabel(this);
    pArrowPixmapLbl = new TypeLabel(this);

    m_pClickLbl->setMinimumSize(125, 40);
    m_pClickLbl->setObjectName("ClickTypeLabel");
    m_pClickLbl->setFocusPolicy(Qt::TabFocus);
    m_pClickLbl->installEventFilter(this);

    DStyle style;   // 设置菜单箭头
    QPixmap pixmap = style.standardIcon(DStyle::StandardPixmap::SP_ReduceElement).pixmap(QSize(10, 10));
    pArrowPixmapLbl->setMinimumHeight(25);
    pArrowPixmapLbl->setPixmap(pixmap);

    DFontSizeManager::instance()->bind(m_pCompressTypeLbl, DFontSizeManager::T5, QFont::DemiBold);

    // 右侧界面
    DLabel *pAdvancedLbl = new DLabel(tr("Advanced Options"), this);
    pAdvancedLbl->setForegroundRole(DPalette::WindowText);
    m_pFileNameEdt = new DLineEdit(this);
    m_pSavePathEdt = new DFileChooserEdit(this);
    m_pAdvancedBtn = new CustomSwitchButton(this);
    m_pEncryptedLbl = new DLabel(tr("Encrypt the archive") + ":", this);
    m_pPasswordEdt = new DPasswordEdit(this);
    m_pListEncryptionLbl = new DLabel(tr("Encrypt the file list too"), this);
    m_pListEncryptionBtn = new CustomSwitchButton(this);
    m_pSplitCkb = new CustomCheckBox(tr("Split to volumes") + ":", this);
    m_pSplitValueEdt = new DDoubleSpinBox(this);
    m_pCompressBtn = new CustomPushButton(tr("Compress"), this);

    m_pFileNameEdt->setMinimumSize(260, 36);    // 配置文件名属性
    QLineEdit *pNameEdt = m_pFileNameEdt->lineEdit();
    pNameEdt->setMaxLength(70);
    m_pFileNameEdt->setText(tr("New Archive"));

    m_pSavePathEdt->setFileMode(DFileDialog::Directory);        // 配置保存路径
    m_pSavePathEdt->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_pSavePathEdt->setMinimumSize(260, 36);

    pAdvancedLbl->setForegroundRole(DPalette::WindowText);

    m_pPasswordEdt->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);   // 密码框默认屏蔽输入法

    m_pEncryptedLbl->setToolTip(tr("Support zip, 7z type only"));   // 设置加密文件提示语
    m_pEncryptedLbl->setForegroundRole(DPalette::WindowText);

    m_pListEncryptionLbl->setToolTip(tr("Support 7z type only"));   // 设置列表加密提示语
    m_pListEncryptionLbl->setEnabled(false);

    m_pCompressBtn->setMinimumSize(340, 36);    // 设置压缩按钮最小尺寸

    // 左侧布局
    QHBoxLayout *pTypeLayout = new QHBoxLayout; // 类型布局
    pTypeLayout->addStretch();
    pTypeLayout->addWidget(m_pCompressTypeLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pTypeLayout->addWidget(pArrowPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pTypeLayout->addStretch();
    m_pClickLbl->setLayout(pTypeLayout);

    QVBoxLayout *pLeftLayout = new QVBoxLayout; // 左侧整体布局
    pLeftLayout->addSpacing(65);
    pLeftLayout->addWidget(m_pTypePixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pLeftLayout->addWidget(m_pClickLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pLeftLayout->addStretch();

    // 右侧布局
    QFormLayout *pFileFormLayout = new QFormLayout();
    pFileFormLayout->addRow(tr("Name") + ":", m_pFileNameEdt);
    pFileFormLayout->addRow(tr("Save to") + ":", m_pSavePathEdt);
    pFileFormLayout->setLabelAlignment(Qt::AlignLeft);
    pFileFormLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    pFileFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QHBoxLayout *pAdvancedLayout = new QHBoxLayout();       // 高级选项布局
    pAdvancedLayout->addWidget(pAdvancedLbl, 0, Qt::AlignLeft);
    pAdvancedLayout->addWidget(m_pAdvancedBtn, 1, Qt::AlignRight);

    QHBoxLayout *pListEncryptionLayout = new QHBoxLayout();    // 列表加密布局
    pListEncryptionLayout->addWidget(m_pListEncryptionLbl, 0, Qt::AlignLeft);
    pListEncryptionLayout->addWidget(m_pListEncryptionBtn, 1, Qt::AlignRight);

    QVBoxLayout *pRightLayout = new QVBoxLayout;    // 右侧内容布局
    pRightLayout->addLayout(pFileFormLayout);
    pRightLayout->addLayout(pAdvancedLayout);
    pRightLayout->addWidget(m_pEncryptedLbl);
    pRightLayout->addWidget(m_pPasswordEdt);
    pRightLayout->addLayout(pListEncryptionLayout);
    pRightLayout->addWidget(m_pSplitCkb);
    pRightLayout->addWidget(m_pSplitValueEdt);
    pRightLayout->addStretch();

    QScrollArea *m_pRightScroll = new QScrollArea(this);    // 右侧滚动区域
    DWidget *pRightWgt = new DWidget(this);
    pRightWgt->setLayout(pRightLayout);
    m_pRightScroll->setFrameShape(QFrame::NoFrame);
    m_pRightScroll->setWidgetResizable(true);
    m_pRightScroll->setMinimumHeight(345);
    m_pRightScroll->setWidget(pRightWgt);

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pCompressBtn, 2);
    pBtnLayout->addStretch(1);

    // 主布局
    QHBoxLayout *pContentLayout = new QHBoxLayout;  // 内容布局
    pContentLayout->addStretch();
    pContentLayout->addLayout(pLeftLayout, 8);
    pContentLayout->addStretch();
    pContentLayout->addWidget(m_pRightScroll, 10);
    pContentLayout->addStretch();
    pContentLayout->setContentsMargins(0, 0, 50, 0);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);   // 主布局
    pMainLayout->addLayout(pContentLayout, 10);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setContentsMargins(12, 6, 20, 20);

    // 初始化菜单
    m_pTypeMenu = new QMenu(this);
    m_pTypeMenu->setMinimumWidth(162);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void CompressSettingPage::initConnections()
{
    connect(m_pClickLbl, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(slotShowRightMenu(QMouseEvent *)));
    connect(m_pCompressTypeLbl, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(slotShowRightMenu(QMouseEvent *)));
    connect(pArrowPixmapLbl, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(slotShowRightMenu(QMouseEvent *)));
    connect(m_pTypeMenu, &DMenu::triggered, this, &CompressSettingPage::slotTypeChanged);
    connect(m_pFileNameEdt, &DLineEdit::textChanged, this, &CompressSettingPage::slotFileNameChanged);
    connect(m_pAdvancedBtn, &DSwitchButton::toggled, this, &CompressSettingPage::slotAdvancedEnabled);
    connect(m_pSplitCkb, &DCheckBox::stateChanged, this, &CompressSettingPage::slotSplitEdtEnabled);
    connect(m_pCompressBtn, &DPushButton::clicked, this, &CompressSettingPage::slotCompressClicked);
}

void CompressSettingPage::setTypeImage(const QString &strType)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp." + strType));

    m_pTypePixmapLbl->setPixmap(icon.pixmap(128, 128));
}

bool CompressSettingPage::checkFileNameVaild(const QString strText)
{
    // 文件名为空返回错误
    if (strText.length() == 0) {
        return false;
    }

    // 文件名过长返回错误
    if (strText.length() > 255) {
        return false;
    }

    // 如果文件名中包干"/"，返回错误
    if (strText.contains(QDir::separator())) {
        return false;
    }

    return true;
}

void CompressSettingPage::setEncryptedEnabled(bool bEnabled)
{
    // 设置加密是否可用
    m_pEncryptedLbl->setEnabled(bEnabled);
    m_pPasswordEdt->setEnabled(bEnabled);

    if (!bEnabled) {
        setListEncryptionEnabled(bEnabled);
    }
}

void CompressSettingPage::slotEchoModeChanged(bool bEchoOn)
{
    // 根据明暗码决定是否屏蔽输入法
    m_pPasswordEdt->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, bEchoOn);
}

/**
 * @brief setListEncryptionEnabled  设置列表加密是否可用
 * @param bEnabled  是否可用
 */
void CompressSettingPage::setListEncryptionEnabled(bool bEnabled)
{
    m_pListEncryptionLbl->setEnabled(bEnabled);
    m_pListEncryptionBtn->setEnabled(bEnabled);
}

void CompressSettingPage::setSplitEnabled(bool bEnabled)
{
    m_pSplitCkb->setEnabled(bEnabled);  // 设置分卷勾选框是否可用

    // 如果不分卷，取消勾选，清空分卷数据
    if (!bEnabled) {
        m_pSplitCkb->setCheckState(Qt::Unchecked);
        m_pSplitValueEdt->setEnabled(bEnabled);
        m_pSplitValueEdt->clear();
    } else {
        m_pSplitValueEdt->setEnabled(m_pSplitCkb->isChecked());
    }
}

void CompressSettingPage::slotShowRightMenu(QMouseEvent *e)
{
    Q_UNUSED(e)

    // 设置菜单弹出位置
    QPoint pos;
    pos.setX(window()->x() + m_pTypePixmapLbl->x() + 60);
    pos.setY(window()->y() + m_pTypePixmapLbl->y() + 240);
    m_pTypeMenu->popup(pos);
}

void CompressSettingPage::slotTypeChanged(QAction *action)
{
    if (action == nullptr)
        return;

    m_strMimeType = action->data().toString();

    QString selectType = action->text();
    setTypeImage(selectType);
    m_pCompressTypeLbl->setText(selectType);

    if (0 == selectType.compare("tar.7z")) {       // tar.7z支持普通/列表加密，不支持分卷
        setEncryptedEnabled(true);
        setListEncryptionEnabled(true);
        setSplitEnabled(false);
    } else if (0 == selectType.compare("7z")) {     // 7z支持普通/列表加密。支持分卷
        setEncryptedEnabled(true);
        setListEncryptionEnabled(true);
        setSplitEnabled(true);
    } else if (0 == selectType.compare("zip")) {    // zip支持普通加密，不支持列表加密，不支持分卷
        setEncryptedEnabled(true);
        setListEncryptionEnabled(false);
        setSplitEnabled(false);
    } else {                                // 其余格式不支持加密，不支持分卷
        setEncryptedEnabled(false);
        setListEncryptionEnabled(false);
        setSplitEnabled(false);
    }
}

void CompressSettingPage::slotFileNameChanged(const QString &strText)
{
    DPalette plt = DApplicationHelper::instance()->palette(m_pFileNameEdt);

    if (!strText.isEmpty()) {
        // 检测文件名合法性
        if (false == checkFileNameVaild(strText)) {
            plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));     // 警告色
        } else {
            plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));      // 正常颜色
        }
    } else {
        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
    }

    m_pFileNameEdt->setPalette(plt);
}

void CompressSettingPage::slotAdvancedEnabled(bool bEnabled)
{
    // 设置控件是否隐藏
    m_pEncryptedLbl->setVisible(bEnabled);
    m_pPasswordEdt->setVisible(bEnabled);
    m_pListEncryptionLbl->setVisible(bEnabled);
    m_pListEncryptionBtn->setVisible(bEnabled);
    m_pSplitCkb->setVisible(bEnabled);
    m_pSplitValueEdt->setVisible(bEnabled);

    // 不启用高级选项时清空界面数据
    if (!bEnabled) {
        m_pPasswordEdt->clear();
        m_pListEncryptionBtn->setChecked(false);
        m_pSplitCkb->setChecked(false);
        m_pSplitValueEdt->clear();
    }
}

void CompressSettingPage::slotSplitEdtEnabled()
{
    // 设置分卷输入框是否可用
    m_pSplitValueEdt->setEnabled(m_pSplitCkb->isChecked());

    if (m_pSplitCkb->isChecked()) {     // 显示单位大小
        QString size = UiTools::humanReadableSize(m_qFileSize, 1);
        m_pSplitValueEdt->setToolTip(tr("Total size: %1").arg(size));
    } else {
        m_pSplitValueEdt->clear();
    }
}

void CompressSettingPage::slotCompressClicked()
{
    qDebug() << "点击了压缩按钮";

    CompressParameter compressInfo;

    compressInfo.strMimeType = m_strMimeType;       // 格式类型
    compressInfo.strArchiveName = m_pFileNameEdt->text() + "." + m_pCompressTypeLbl->text();;   // 压缩包名称
    compressInfo.strTargetPath = m_pSavePathEdt->text();    // 压缩包保存路径
    compressInfo.strPassword = m_pPasswordEdt->lineEdit()->text();
    compressInfo.bEncryption = !(compressInfo.strPassword.isEmpty());     // 是否加密
    compressInfo.strEncryptionMethod = "AES256";        // 加密算法
    compressInfo.bHeaderEncryption = m_pListEncryptionBtn->isChecked();     // 是否列表加密
    compressInfo.bSplit = m_pSplitCkb->isChecked();     // 是否分卷
    compressInfo.iVolumeSize = static_cast< int >(m_pSplitValueEdt->value() * 1024);    // 分卷大小

    // 压缩等级
    if ("application/x-tar" == compressInfo.strMimeType || "application/x-tarz" == compressInfo.strMimeType) {
        compressInfo.iCompressionLevel = -1;  //-1 is unuseful
    } else if ("application/zip" == compressInfo.strMimeType) {
        compressInfo.iCompressionLevel = 3;  // 1:Extreme 3:Fast 4:Standard
    } else {
        compressInfo.iCompressionLevel = 6;  // 6 is default
    }

    // 检测此压缩包名称是否存在
    QFileInfo fileInfo(compressInfo.strTargetPath + QDir::separator() + compressInfo.strArchiveName);
    if (fileInfo.exists()) {
        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("Another file with the same name already exists, replace it?"), tr("Cancel"), DDialog::ButtonNormal, tr("Replace"), DDialog::ButtonWarning);
        if (iResult == 1) {     // 如果点击替换，先移除本地压缩包
            QFile file(fileInfo.filePath());
            file.remove();
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    }

    emit signalCompressClicked(QVariant::fromValue(compressInfo));

}
