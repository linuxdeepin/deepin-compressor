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
#include "queries.h"

#include <DDialog>
#include <DPasswordEdit>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DLabel>

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QPointer>
#include <QUrl>
#include <QBoxLayout>
#include <QImageReader>
#include <QDebug>
#include <QMainWindow>


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

Query::Query()
{
}

QVariant Query::response() const
{
    return m_data.value(QStringLiteral("response"));
}

int Query::execDialog()
{
    //todo 20191125
    return 0;
}

void Query::setParent(DMainWindow *pParent)
{
    m_pParent = pParent;
}

QString Query::toShortString(QString strSrc, int limitCounts, int left)
{
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    return displayName;
}

void Query::waitForResponse()
{
    QMutexLocker locker(&m_responseMutex);
    //if there is no response set yet, wait
    if (!m_data.contains(QStringLiteral("response"))) {
        m_responseCondition.wait(&m_responseMutex);
    }
}

void Query::setResponse(const QVariant &response)
{
    m_data[QStringLiteral("response")] = response;
    m_responseCondition.wakeAll();
}

OverwriteQuery::OverwriteQuery(const QString &filename) :
    m_noRenameMode(false),
    m_multiMode(true)
{
    m_data[QStringLiteral("filename")] = filename;
}

void OverwriteQuery::colorRoleChange(QWidget *widget, DPalette::ColorRole ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(widget);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(widget, palette);
}

void OverwriteQuery::colorTypeChange(QWidget *widget, DPalette::ColorType ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(widget);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(widget, palette);
}

bool OverwriteQuery::applyAll()
{
    return m_bApplyAll;
}

void OverwriteQuery::execute()
{

    QUrl sourceUrl = QUrl::fromLocalFile(QDir::cleanPath(m_data.value(QStringLiteral("filename")).toString()));

    QString path = sourceUrl.toString();
    if (path.contains("file://")) {
        path.remove("file://");
    }

    if (path.contains("file:")) {
        path.remove("file:");
    }

    QFileInfo file(path);

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);

    dialog->setMinimumSize(QSize(380, 190));
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(280, 20));
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    int limitCounts = 16;
    int left = 8, right = 8;
    QString fileName = file.fileName();
    QString displayName = "";
    displayName = fileName.length() > limitCounts ? fileName.left(left) + "..." + fileName.right(right) : fileName;
    strlabel->setText(displayName);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(154, 20));
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));

    dialog->addButton(QObject::tr("Skip"));
    dialog->addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);

    DCheckBox *checkbox = new DCheckBox;
    checkbox->setStyleSheet("QCheckBox::indicator {width: 21px; height: 21px;}");

    DLabel *checkLabel = new DLabel(QObject::tr("Apply to all"));
    checkLabel->setMinimumSize(QSize(98, 20));
    DFontSizeManager::instance()->bind(checkLabel, DFontSizeManager::T6, QFont::Medium);

    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        colorRoleChange(strlabel, DPalette::ToolTipText, 0.7);
        colorRoleChange(strlabel2, DPalette::ToolTipText, 1);
        colorRoleChange(checkLabel, DPalette::Text, 1);
        colorRoleChange(checkbox, DPalette::ToolTipText, 0.7);
    }
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        colorTypeChange(strlabel, DPalette::TextLively, 0.7);
        colorTypeChange(strlabel2, DPalette::TextLively, 1);
        colorRoleChange(checkLabel, DPalette::Text, 1);
        colorTypeChange(checkbox, DPalette::TextLively, 0.7);
    }

    QHBoxLayout *checkLayout = new QHBoxLayout;
    checkLayout->addStretch();
    checkLayout->addWidget(checkbox);
    checkLayout->addWidget(checkLabel);
    checkLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);

//    mainlayout->addWidget(checkbox, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addLayout(checkLayout);
//    mainlayout->SetFixedSize(QSize());
    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();
    ret = mode;
    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else if (0 == mode) {
        if (checkbox->isChecked()) {
            setResponse(Result_AutoSkip);
        } else {
            setResponse(Result_Skip);
        }
    } else if (1 == mode) {
        if (checkbox->isChecked()) {
            setResponse(Result_OverwriteAll);
        } else {
            setResponse(Result_Overwrite);
        }
    }

    m_bApplyAll = checkbox->isChecked();

    delete dialog;
    //QApplication::restoreOverrideCursor();
}
int OverwriteQuery::getExecuteReturn()
{
    return ret;
}
bool OverwriteQuery::responseCancelled()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_CANCEL;
}
bool OverwriteQuery::responseOverwriteAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_OVERWRITE_ALL;
}
bool OverwriteQuery::responseOverwrite()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_OVERWRITE;
}

bool OverwriteQuery::responseRename()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_RENAME;
}

bool OverwriteQuery::responseSkip()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_SKIP;
}

bool OverwriteQuery::responseAutoSkip()
{
    return m_data.value(QStringLiteral("response")).toInt() == R_AUTO_SKIP;
}

QString OverwriteQuery::newFilename()
{
    return m_data.value(QStringLiteral("newFilename")).toString();
}

void OverwriteQuery::setNoRenameMode(bool enableNoRenameMode)
{
    m_noRenameMode = enableNoRenameMode;
}

bool OverwriteQuery::noRenameMode()
{
    return m_noRenameMode;
}

void OverwriteQuery::setMultiMode(bool enableMultiMode)
{
    m_multiMode = enableMultiMode;
}

bool OverwriteQuery::multiMode()
{
    return m_multiMode;
}

PasswordNeededQuery::PasswordNeededQuery(const QString &archiveFilename, bool incorrectTryAgain)
{
    m_data[QStringLiteral("archiveFilename")] = archiveFilename;
    m_data[QStringLiteral("incorrectTryAgain")] = incorrectTryAgain;
}

void PasswordNeededQuery::execute()
{
    // If we are being called from the KPart, the cursor is probably Qt::WaitCursor
    // at the moment (#231974)
//    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

//    DDialog* dlg = new DDialog;
//    dlg->setMessage(QObject::tr("This file is encrypted, please enter the password"));


//    const bool notCancelled = dlg->exec();
//    const QString password = "";

//    m_data[QStringLiteral("password")] = password;
//    setResponse(notCancelled && !password.isEmpty());

    qDebug() << m_data[QStringLiteral("archiveFilename")];

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);

    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel(dialog);
    strlabel->setFixedSize(300, 20);
    strlabel->setForegroundRole(DPalette::WindowText);
    strlabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::DemiBold);
    QString archiveFullPath = m_data[QStringLiteral("archiveFilename")].toString();
    QString fileName = toShortString(archiveFullPath.mid(archiveFullPath.lastIndexOf('/') + 1), 22, 11);
    strlabel->setText(fileName);
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setToolTip(archiveFullPath);

    DLabel *strlabel2 = new DLabel(dialog);
    strlabel2->setFixedHeight(20);
    strlabel2->setForegroundRole(DPalette::WindowText);
    strlabel2->setWordWrap(true);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Normal);
    strlabel2->setText(tr("Encrypted file, please enter the password"));
    strlabel2->setAlignment(Qt::AlignCenter);

    DPasswordEdit *passwordedit = new DPasswordEdit(dialog);
    passwordedit->setFixedWidth(280);

    dialog->addButton(QObject::tr("OK"));
    dialog->getButton(0)->setEnabled(false);
    //确保输入的密码不为空
    connect(passwordedit, &DPasswordEdit::textChanged, passwordedit, [ & ]() {
        if (passwordedit->text().isEmpty()) {
            dialog->getButton(0)->setEnabled(false);
        } else {
            dialog->getButton(0)->setEnabled(true);
        }
    });

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(passwordedit, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);
    QRect mainWindowGeometr = getMainWindow()->geometry();
    dialog->move(mainWindowGeometr.topLeft().x() + (mainWindowGeometr.width() - dialog->width()) / 2, mainWindowGeometr.topLeft().y() - TITLE_FIXED_HEIGHT + (mainWindowGeometr.height() - dialog->height()) / 2); //居中显示
    const int mode = dialog->exec();


    m_data[QStringLiteral("password")] = passwordedit->text();

    delete dialog;

    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else {
        setResponse(Result_Skip);
    }
}

QString PasswordNeededQuery::password()
{
    return m_data.value(QStringLiteral("password")).toString();
}

bool PasswordNeededQuery::responseCancelled()
{
    return !m_data.value(QStringLiteral("response")).toBool();
}

WrongPasswordQuery::WrongPasswordQuery(const QString &archiveFilename, bool incorrectTryAgain)
{
    m_data[QStringLiteral("archiveFilename")] = archiveFilename;
    m_data[QStringLiteral("incorrectTryAgain")] = incorrectTryAgain;
}

void WrongPasswordQuery::execute()
{
    // If we are being called from the KPart, the cursor is probably Qt::WaitCursor
    // at the moment (#231974)

    qDebug() << m_data[QStringLiteral("archiveFilename")];
    QFileInfo file(m_data[QStringLiteral("archiveFilename")].toString());

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);

    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel(dialog);
    strlabel->setFixedHeight(20);
    strlabel->setForegroundRole(DPalette::TextTitle);

    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setText(file.fileName());

    DLabel *strlabel2 = new DLabel(dialog);
    strlabel2->setFixedHeight(20);
    strlabel2->setForegroundRole(DPalette::TextWarning);

    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);
    strlabel2->setText(QObject::tr("Wrong password"));

    dialog->addButton(QObject::tr("OK"));


    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    dialog->exec();
    delete dialog;

    setResponse(Result_Cancel);
}

QString WrongPasswordQuery::password()
{
    return m_data.value(QStringLiteral("password")).toString();
}

bool WrongPasswordQuery::responseCancelled()
{
    return !m_data.value(QStringLiteral("response")).toBool();
}

LoadCorruptQuery::LoadCorruptQuery(const QString &archiveFilename)
{
    m_data[QStringLiteral("archiveFilename")] = archiveFilename;
}

void LoadCorruptQuery::execute()
{

}

bool LoadCorruptQuery::responseYes()
{
    return true;
}

ContinueExtractionQuery::ContinueExtractionQuery(const QString &error, const QString &archiveEntry)
    : m_chkDontAskAgain("Don't ask again.")
{
    m_data[QStringLiteral("error")] = error;
    m_data[QStringLiteral("archiveEntry")] = archiveEntry;
}

void ContinueExtractionQuery::execute()
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    QApplication::restoreOverrideCursor();
}

bool ContinueExtractionQuery::responseCancelled()
{
    return (m_data.value(QStringLiteral("response")).toInt() == QMessageBox::Cancel);
}

bool ContinueExtractionQuery::dontAskAgain()
{
    return m_chkDontAskAgain.isChecked();
}


