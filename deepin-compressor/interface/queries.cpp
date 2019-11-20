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
#include <QCheckBox>
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
    return m_data.value(QStringLiteral( "response" ));
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
    m_data[QStringLiteral( "response" )] = response;
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
    if(path.contains("file://"))
    {
        path.remove("file://");
    }

    QFileInfo file(path);

    DDialog* dialog = new DDialog;
    QPixmap pixmap = renderSVG(":/images/warning.svg", QSize(64, 64));
    dialog->setIconPixmap(pixmap);

    DPalette pa;

    DLabel* strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    pa = DApplicationHelper::instance()->palette(strlabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::WindowText));
    strlabel->setPalette(pa);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::Medium);
    strlabel->setFont(font);
    strlabel->setText(file.fileName());
    DLabel* strlabel2 = new DLabel;
    pa = DApplicationHelper::instance()->palette(strlabel2);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextWarning));
    strlabel2->setPalette(pa);
    strlabel2->setFixedHeight(20);

    font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    strlabel2->setFont(font);
    strlabel2->setText(QObject::tr("The file already exists. How to deal with it?"));

    dialog->addButton(QObject::tr("Jump over"));
    dialog->addButton(QObject::tr("Rover"));

    DCheckBox* checkbox = new DCheckBox;
    checkbox->setText(QObject::tr("Apply to all files"));

    QVBoxLayout* mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(checkbox, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    DWidget* widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();

    if(-1 == mode)
    {
        setResponse(Result_Cancel);
    }
    else if(0 == mode)
    {
        if(checkbox->isChecked())
        {
            setResponse(Result_AutoSkip);
        }
        else {
            setResponse(Result_Skip);
        }
    }
    else if(1 == mode)
    {
        if(checkbox->isChecked())
        {
            setResponse(Result_OverwriteAll);
        }
        else {
            setResponse(Result_Overwrite);
        }
    }

    QApplication::restoreOverrideCursor();
}

bool OverwriteQuery::responseCancelled()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_CANCEL;
}
bool OverwriteQuery::responseOverwriteAll()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_OVERWRITE_ALL;
}
bool OverwriteQuery::responseOverwrite()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_OVERWRITE;
}

bool OverwriteQuery::responseRename()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_RENAME;
}

bool OverwriteQuery::responseSkip()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_SKIP;
}

bool OverwriteQuery::responseAutoSkip()
{
    return m_data.value(QStringLiteral( "response" )).toInt() == R_AUTO_SKIP;
}

QString OverwriteQuery::newFilename()
{
    return m_data.value(QStringLiteral( "newFilename" )).toString();
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

PasswordNeededQuery::PasswordNeededQuery(const QString& archiveFilename, bool incorrectTryAgain)
{
    m_data[QStringLiteral( "archiveFilename" )] = archiveFilename;
    m_data[QStringLiteral( "incorrectTryAgain" )] = incorrectTryAgain;
}

void PasswordNeededQuery::execute()
{
    // If we are being called from the KPart, the cursor is probably Qt::WaitCursor
    // at the moment (#231974)
//    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

//    QPointer<KPasswordDialog> dlg = new KPasswordDialog;
//    dlg.data()->setPrompt(xi18nc("@info", "The archive <filename>%1</filename> is password protected. Please enter the password.",
//                                 m_data.value(QStringLiteral("archiveFilename")).toString()));

//    if (m_data.value(QStringLiteral("incorrectTryAgain")).toBool()) {
//        dlg.data()->showErrorMessage(i18n("Incorrect password, please try again."), KPasswordDialog::PasswordError);
//    }

//    const bool notCancelled = dlg.data()->exec();
//    const QString password = dlg.data()->password();

//    m_data[QStringLiteral("password")] = password;
//    setResponse(notCancelled && !password.isEmpty());

//    QApplication::restoreOverrideCursor();

//    delete dlg.data();
}

QString PasswordNeededQuery::password()
{
    return m_data.value(QStringLiteral( "password" )).toString();
}

bool PasswordNeededQuery::responseCancelled()
{
    return !m_data.value(QStringLiteral( "response" )).toBool();
}

LoadCorruptQuery::LoadCorruptQuery(const QString& archiveFilename)
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

bool LoadCorruptQuery::responseYes() {
//    return (m_data.value(QStringLiteral("response")).toInt() == KMessageBox::Yes);
    return true;
}

ContinueExtractionQuery::ContinueExtractionQuery(const QString& error, const QString& archiveEntry)
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

bool ContinueExtractionQuery::responseCancelled() {
    return (m_data.value(QStringLiteral("response")).toInt() == QMessageBox::Cancel);
}

bool ContinueExtractionQuery::dontAskAgain() {
    return m_chkDontAskAgain.isChecked();
}


