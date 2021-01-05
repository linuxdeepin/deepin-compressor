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
#include "queries.h"

#include <DMainWindow>
#include <DDialog>
#include <DFontSizeManager>
#include <DLabel>
#include <DCheckBox>
#include <DPasswordEdit>

#include <QApplication>
#include <QImageReader>
#include <QLayout>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

DMainWindow *getMainWindow()
{
    foreach (QWidget *w, QApplication::allWidgets()) {
        QMainWindow *pWindow = qobject_cast<QMainWindow *>(w);
        if (pWindow) {
            DMainWindow *pp = qobject_cast<DMainWindow *>(pWindow);
            return pp;
        }
    }

    return nullptr;
}

static QPixmap renderSVG(const QString &filePath, const QSize &size)
{
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    return pixmap;
}



Query::Query(QObject *parent)
    : QObject(parent)
{

}

Query::~Query()
{

}

void Query::setParent(QWidget *pParent)
{
    m_pParent = pParent;
}

void Query::waitForResponse()
{
    QMutexLocker locker(&m_responseMutex);
    // 如果没有任何选择，等待响应
    if (!m_data.contains(QStringLiteral("response"))) {
        m_responseCondition.wait(&m_responseMutex);
    }
}

void Query::setResponse(const QVariant &response)
{
    // 唤醒响应
    m_data[QStringLiteral("response")] = response;
    m_responseCondition.wakeAll();
}

QString Query::toShortString(QString strSrc, int limitCounts, int left)
{
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    return displayName;
}


OverwriteQuery::OverwriteQuery(const QString &filename, QObject *parent)
    : Query(parent)
{
    m_data[QStringLiteral("filename")] = filename;
}

OverwriteQuery::~OverwriteQuery()
{

}

void OverwriteQuery::execute()
{
    // 文件名处理
    QUrl sourceUrl = QUrl::fromLocalFile(QDir::cleanPath(m_data.value(QStringLiteral("filename")).toString()));

    QString path = sourceUrl.toString();
    if (path.contains("file://")) {
        path.remove("file://");
    }

    if (path.contains("file:")) {
        path.remove("file:");
    }

    QFileInfo file(path);

    // 获取父窗口（居中显示）
    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);
    dialog->setAccessibleName("Overwrite_dialog");
    dialog->setMinimumSize(QSize(380, 190));

    // 设置对话框图标
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    // 显示文件名
    DLabel *pFileNameLbl = new DLabel;
    pFileNameLbl->setMinimumSize(QSize(280, 20));
    pFileNameLbl->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(pFileNameLbl, DFontSizeManager::T6, QFont::Normal);

    int limitCounts = 16;
    int left = 8, right = 8;
    QString fileName = file.fileName();
    QString displayName = "";
    displayName = fileName.length() > limitCounts ? fileName.left(left) + "..." + fileName.right(right) : fileName;
    pFileNameLbl->setText(displayName);

    // 显示提示语
    DLabel *pTipLbl = new DLabel;
    pTipLbl->setMinimumSize(QSize(154, 20));
    pTipLbl->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(pTipLbl, DFontSizeManager::T6, QFont::Medium);
    pTipLbl->setText(QObject::tr("Another file with the same name already exists, replace it?"));

    // 应用到全部勾选
    DCheckBox *pApplyAllCkb = new DCheckBox;
    pApplyAllCkb->setAccessibleName("Applyall_btn");
    pApplyAllCkb->setStyleSheet("QCheckBox::indicator {width: 21px; height: 21px;}");

    DLabel *pApplyAllLbl = new DLabel(QObject::tr("Apply to all"));
    pApplyAllLbl->setMinimumSize(QSize(98, 20));
    DFontSizeManager::instance()->bind(pApplyAllLbl, DFontSizeManager::T6, QFont::Medium);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    // 控件主题样式
    if (themeType == DGuiApplicationHelper::LightType) {    // 浅色
        setWidgetColor(pFileNameLbl, DPalette::ToolTipText, 0.7);
        setWidgetColor(pTipLbl, DPalette::ToolTipText, 1);
        setWidgetColor(pApplyAllCkb, DPalette::ToolTipText, 0.7);
    } else if (themeType == DGuiApplicationHelper::DarkType) {  // 深色
        setWidgetType(pFileNameLbl, DPalette::TextLively, 0.7);
        setWidgetType(pTipLbl, DPalette::TextLively, 1);
        setWidgetType(pApplyAllCkb, DPalette::TextLively, 0.7);
    }

    // 按钮
    dialog->addButton(QObject::tr("Skip"));
    dialog->addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);

    // 布局
    QHBoxLayout *checkLayout = new QHBoxLayout;
    checkLayout->addStretch();
    checkLayout->addWidget(pApplyAllCkb);
    checkLayout->addWidget(pApplyAllLbl);
    checkLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(pTipLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(pFileNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    mainlayout->addLayout(checkLayout);
    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    //setTabOrder需放在布局最后，否则不生效
    dialog->setTabOrder(pApplyAllCkb, dialog->getButton(0));
    dialog->setTabOrder(dialog->getButton(0), dialog->getButton(1));

    // 操作结果
    const int mode = dialog->exec();

    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else if (0 == mode) {
        if (pApplyAllCkb->isChecked()) {
            setResponse(Result_SkipAll);
        } else {
            setResponse(Result_Skip);
        }
    } else if (1 == mode) {
        if (pApplyAllCkb->isChecked()) {
            setResponse(Result_OverwriteAll);
        } else {
            setResponse(Result_Overwrite);
        }
    }

    delete dialog;
}

bool OverwriteQuery::responseCancelled()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Cancel;
}

bool OverwriteQuery::responseSkip()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Skip;
}

bool OverwriteQuery::responseSkipAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_SkipAll;
}

bool OverwriteQuery::responseOverwrite()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Overwrite;
}

bool OverwriteQuery::responseOverwriteAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_OverwriteAll;
}

void OverwriteQuery::setWidgetColor(QWidget *pWgt, DPalette::ColorRole ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(pWgt, palette);
}

void OverwriteQuery::setWidgetType(QWidget *pWgt, DPalette::ColorType ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(pWgt, palette);
}



PasswordNeededQuery::PasswordNeededQuery(const QString &strFileName, QObject *parent)
    : Query(parent)
{
    m_data[QStringLiteral("fileName")] = strFileName;
}

PasswordNeededQuery::~PasswordNeededQuery()
{

}

void PasswordNeededQuery::execute()
{
    // 获取父窗口（居中显示）
    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);
    dialog->setAccessibleName("PasswordNeeded_dialog");
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    // 加密文件名显示
    DLabel *pFileNameLbl = new DLabel(dialog);
    pFileNameLbl->setFixedSize(300, 20);
    pFileNameLbl->setForegroundRole(DPalette::WindowText);
    pFileNameLbl->setWordWrap(true);
    DFontSizeManager::instance()->bind(pFileNameLbl, DFontSizeManager::T6, QFont::DemiBold);
    QString archiveFullPath = m_data[QStringLiteral("fileName")].toString();
    QString fileName = toShortString(archiveFullPath.mid(archiveFullPath.lastIndexOf('/') + 1), 22, 11);
    pFileNameLbl->setText(fileName);
    pFileNameLbl->setAlignment(Qt::AlignCenter);
    pFileNameLbl->setToolTip(archiveFullPath);

    // 提示语显示
    DLabel *pTipLbl = new DLabel(dialog);
    pTipLbl->setFixedWidth(340); //修复英文环境下提示语显示不全
    pTipLbl->setForegroundRole(DPalette::WindowText);
    pTipLbl->setWordWrap(true);
    DFontSizeManager::instance()->bind(pTipLbl, DFontSizeManager::T6, QFont::Normal);
    pTipLbl->setText(tr("Encrypted file, please enter the password"));
    pTipLbl->setAlignment(Qt::AlignCenter);

    // 密码框
    DPasswordEdit *passwordedit = new DPasswordEdit(dialog);
    passwordedit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false); //隐藏密码时不能输入中文
    passwordedit->setFocusPolicy(Qt::StrongFocus);
    passwordedit->setFixedWidth(280);

    dialog->addButton(QObject::tr("Cancel"), true, DDialog::ButtonNormal);
    dialog->addButton(QObject::tr("OK"), true, DDialog::ButtonRecommend);
    dialog->getButton(1)->setEnabled(false);
    //确保输入的密码不为空
    connect(passwordedit, &DPasswordEdit::textChanged, passwordedit, [&]() {
        if (passwordedit->text().isEmpty()) {
            dialog->getButton(1)->setEnabled(false);
        } else {
            dialog->getButton(1)->setEnabled(true);
        }
    });

    //隐藏密码时不能输入中文,显示密码时可以输入中文
    connect(passwordedit, &DPasswordEdit::echoModeChanged, passwordedit, [&](bool echoOn) {
        passwordedit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, echoOn);
    });

    // 布局
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(pFileNameLbl, 0, Qt::AlignCenter);
    mainlayout->addWidget(pTipLbl, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(passwordedit, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);
    QRect mainWindowGeometr = getMainWindow()->geometry();
    dialog->move(mainWindowGeometr.topLeft().x() + (mainWindowGeometr.width() - dialog->width()) / 2, mainWindowGeometr.topLeft().y() - 50 + (mainWindowGeometr.height() - dialog->height()) / 2); //居中显示
    const int mode = dialog->exec();

    m_data[QStringLiteral("password")] = passwordedit->text();

    if (-1 == mode || 0 == mode) {
        setResponse(Result_Cancel);
    } else {
        setResponse(Result_Skip);
    }

    delete dialog;
}

bool PasswordNeededQuery::responseCancelled()
{
    return !m_data.value(QStringLiteral("response")).toBool();
}

QString PasswordNeededQuery::password()
{
    return m_data.value(QStringLiteral("password")).toString();
}
