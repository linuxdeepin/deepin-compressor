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
#include "queries.h"


#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QPointer>
#include <QUrl>
#include <DDialog>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DLabel>
#include <QBoxLayout>
#include <QImageReader>
#include <QDebug>
#include <DPasswordEdit>

DWIDGET_USE_NAMESPACE

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

void OverwriteQuery::execute()
{

    QUrl sourceUrl = QUrl::fromLocalFile(QDir::cleanPath(m_data.value(QStringLiteral("filename")).toString()));

    QString path = sourceUrl.toString();
    if (path.contains("file://")) {
        path.remove("file://");
    }

    QFileInfo file(path);

    DDialog *dialog = new DDialog;
    QPixmap pixmap = renderSVG(":/images/warning.svg", QSize(64, 64));
    dialog->setIconPixmap(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    strlabel->setForegroundRole(DPalette::WindowText);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
//    font.setWeight(QFont::Medium);
//    strlabel->setFont(font);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setText(file.fileName());
    DLabel *strlabel2 = new DLabel;
    strlabel2->setFixedHeight(20);
    strlabel2->setForegroundRole(DPalette::TextWarning);
//    font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
//    strlabel2->setFont(font);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setText(QObject::tr("The file already exists. How to deal with it?"));
    dialog->addButton(QObject::tr("Skip"));
    dialog->addButton(QObject::tr("Override"));

    DCheckBox *checkbox = new DCheckBox;
    checkbox->setText(QObject::tr("Apply to all"));

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(checkbox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    DWidget *widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();

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

    QApplication::restoreOverrideCursor();
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

    qDebug()<<m_data[QStringLiteral("archiveFilename")];
    DDialog *dialog = new DDialog;
    QPixmap pixmap = renderSVG(":/images/warning.svg", QSize(64, 64));
    dialog->setIconPixmap(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    strlabel->setForegroundRole(DPalette::WindowText);

    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setText(QObject::tr("This file is encrypted, please enter the password"));

    DPasswordEdit* passwordedit = new DPasswordEdit;
    passwordedit->setFixedWidth(280);

    dialog->addButton(QObject::tr("OK"));


    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(passwordedit, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();

    const QString password = "";

    m_data[QStringLiteral("password")] = passwordedit->text();

    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else {
        setResponse(Result_Skip);
    }


    delete dialog;
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

    qDebug()<<m_data[QStringLiteral("archiveFilename")];
    QFileInfo file(m_data[QStringLiteral("archiveFilename")].toString());
    DDialog *dialog = new DDialog;
    QPixmap pixmap = renderSVG(":/images/warning.svg", QSize(64, 64));
    dialog->setIconPixmap(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    strlabel->setForegroundRole(DPalette::TextTitle);

    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setText(file.fileName());

    DLabel *strlabel2 = new DLabel;
    strlabel2->setFixedHeight(20);
    strlabel2->setForegroundRole(DPalette::TextWarning);

    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);
    strlabel2->setText(QObject::tr("Wrong password!"));

    dialog->addButton(QObject::tr("OK"));


    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    dialog->exec();

    setResponse(Result_Cancel);


    delete dialog;
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
//    qCDebug(ARK) << "Executing prompt";
//    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

//    setResponse(KMessageBox::warningYesNo(nullptr,
//                                          xi18nc("@info", "The archive you're trying to open is corrupt.<nl/>"
//                                                 "Some files may be missing or damaged."),
//                                          i18nc("@title:window", "Corrupt archive"),
//                                          KGuiItem(i18nc("@action:button", "Open as Read-Only")),
//                                          KGuiItem(i18nc("@action:button", "Don't Open"))));
//    QApplication::restoreOverrideCursor();
}

bool LoadCorruptQuery::responseYes()
{
//    return (m_data.value(QStringLiteral("response")).toInt() == KMessageBox::Yes);
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

//    QMessageBox box(QMessageBox::Warning,
//                    i18n("Error during extraction"),
//                    xi18n("Extraction of the entry:<nl/>"
//                          "    <filename>%1</filename><nl/>"
//                          "failed with the error message:<nl/>    %2<nl/><nl/>"
//                          "Do you want to continue extraction?<nl/>", m_data.value(QStringLiteral("archiveEntry")).toString(),
//                          m_data.value(QStringLiteral("error")).toString()),
//                    QMessageBox::Yes|QMessageBox::Cancel);
//    box.setCheckBox(&m_chkDontAskAgain);
//    setResponse(box.exec());
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


