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
#include <DApplicationHelper>
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

Query::Query()
{
}

void Query::setParent(QWidget *pParent)
{
    m_pParent = pParent;
}

QWidget *Query::getParent()
{
    return m_pParent;
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

QVariant Query::getResponse() const
{
    return m_data.value(QStringLiteral("response"));
}

QString Query::toShortString(QString strSrc, int limitCounts, int left)
{
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    return displayName;
}

OverwriteQuery::OverwriteQuery(const QString &filename)
    : m_noRenameMode(false)
    , m_multiMode(true)
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

    if (path.contains("file:")) {
        path.remove("file:");
    }

    QFileInfo file(path);

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);
    dialog->setMinimumSize(QSize(380, 190));
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(280, 20));
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    QString displayName = m_data[QStringLiteral("filename")].toString();
    strlabel->setToolTip(displayName);
    displayName = toShortString(displayName);
    strlabel->setText(displayName);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(154, 20));
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));
    strlabel2->setForegroundRole(DPalette::ToolTipText);

    dialog->addButton(QObject::tr("Skip"));
    dialog->addButton(QObject::tr("Replace"), true, DDialog::ButtonWarning);

    DCheckBox *checkbox = new DCheckBox;
    checkbox->setStyleSheet("QCheckBox::indicator {width: 14px; height: 14px;}");

    DLabel *checkLabel = new DLabel(QObject::tr("Apply to all"));
    checkLabel->setMinimumSize(QSize(98, 20));
    DFontSizeManager::instance()->bind(checkLabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *checkLayout = new QHBoxLayout;
    checkLayout->addStretch();
    checkLayout->addWidget(checkbox);
    checkLayout->addWidget(checkLabel);
    checkLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    mainlayout->addLayout(checkLayout);
    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();
    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else if (0 == mode) {
        if (checkbox->isChecked()) {
            setResponse(Result_SkipAll);
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

    m_dialogMode = mode;
    m_applyAll = checkbox->isChecked();
    delete dialog;
}

int OverwriteQuery::getExecuteReturn()
{
    return m_dialogMode;
}

bool OverwriteQuery::getResponseCancell()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Cancel;
}

bool OverwriteQuery::getResponseOverwrite()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Overwrite;
}

bool OverwriteQuery::getResponseOverwriteAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_OverwriteAll;
}

bool OverwriteQuery::getResponseRename()
{
    return m_data.value(QStringLiteral("response")).toInt() ==  Result_Rename;
}

bool OverwriteQuery::getResponseSkip()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Skip;
}

bool OverwriteQuery::getResponseSkipAll()
{
    return m_data.value(QStringLiteral("response")).toInt() ==  Result_SkipAll;
}

QString OverwriteQuery::getNewFilename()
{
    return m_data.value(QStringLiteral("newFilename")).toString();
}

bool OverwriteQuery::getApplyAll()
{
    return m_applyAll;
}

PasswordNeededQuery::PasswordNeededQuery(const QString &archiveFilename, bool incorrectTryAgain)
{
    m_data[QStringLiteral("archiveFilename")] = archiveFilename;
    m_data[QStringLiteral("incorrectTryAgain")] = incorrectTryAgain;
}

void PasswordNeededQuery::execute()
{
    qDebug() << m_data[QStringLiteral("archiveFilename")];

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);
    dialog->setMinimumSize(QSize(380, 180));
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(293, 20));
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setForegroundRole(DPalette::ToolTipText);

    QString displayName = m_data[QStringLiteral("archiveFilename")].toString();
    strlabel->setToolTip(displayName);
    displayName = toShortString(displayName);
    strlabel->setText(tr("%1 is encrypted, please enter the decompression password").arg(displayName));
    strlabel->setWordWrap(true);

    DPasswordEdit *passwordEdit = new DPasswordEdit(dialog);
    passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false); //隐藏密码时不能输入中文
    passwordEdit->setFocusPolicy(Qt::StrongFocus);
    passwordEdit->setMinimumSize(360, 36);

    dialog->addButton(QObject::tr("Cancel"));
    dialog->addButton(QObject::tr("Confirm"), true, DDialog::ButtonRecommend);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(passwordEdit, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    DWidget *widget = new DWidget(dialog);
    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    //确保输入的密码不为空
    connect(passwordEdit, &DPasswordEdit::textChanged, passwordEdit, [&]() {
        if (passwordEdit->text().isEmpty()) {
            dialog->getButton(0)->setEnabled(false);
        } else {
            dialog->getButton(0)->setEnabled(true);
        }
    });

    //隐藏密码时不能输入中文,显示密码时可以输入中文
    connect(passwordEdit, &DPasswordEdit::echoModeChanged, passwordEdit, [&](bool echoOn) {
        passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, echoOn);
    });

    const int mode = dialog->exec();

    m_data[QStringLiteral("password")] = passwordEdit->text();

    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else {
        setResponse(Result_Skip);
    }

    delete dialog;
}

AddCompressUsePasswordQuery::AddCompressUsePasswordQuery()
{

}

void AddCompressUsePasswordQuery::execute()
{
    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    DDialog *dialog = new DDialog(m_pParent);
    dialog->setMinimumSize(QSize(380, 165));
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setMinimumSize(QSize(293, 20));
    strlabel->setText(tr("Add files to the current archive"));
    strlabel->setWordWrap(true);
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setForegroundRole(DPalette::ToolTipText);

    DCheckBox *pwdCheckbox = new DCheckBox;
    pwdCheckbox->setStyleSheet("QCheckBox::indicator {width: 14px; height: 14px;}");

    DLabel *pwdCheckLabel = new DLabel(QObject::tr("Use password"));
    pwdCheckLabel->setMinimumSize(QSize(56, 20));
    DFontSizeManager::instance()->bind(pwdCheckLabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *usePwdLayout = new QHBoxLayout(dialog);
    usePwdLayout->addStretch();
    usePwdLayout->addWidget(pwdCheckbox);
    usePwdLayout->addWidget(pwdCheckLabel);
    usePwdLayout->addStretch();

    dialog->addButton(QObject::tr("Cancel"));
    dialog->addButton(QObject::tr("Confirm"), true, DDialog::ButtonRecommend);

    DPasswordEdit *pwdLineEdit = new DPasswordEdit;
    QLineEdit *edit = pwdLineEdit->lineEdit();
    edit->setPlaceholderText(tr("Please input password"));
    pwdLineEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false); //隐藏密码时不能输入中文
    pwdLineEdit->setFocusPolicy(Qt::StrongFocus);
    pwdLineEdit->setMinimumSize(360, 36);

    QVBoxLayout *mainlayout = new QVBoxLayout(dialog);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addLayout(usePwdLayout);

    connect(pwdCheckbox, &DCheckBox::stateChanged, this, [ = ] {
        if (pwdCheckbox->checkState() == Qt::Checked)
        {
            pwdLineEdit->setEnabled(true);
            dialog->setMinimumSize(QSize(380, 216));
            mainlayout->addWidget(pwdLineEdit);
        } else
        {
            pwdLineEdit->setEnabled(false);
        }
    });

    DWidget *widget = new DWidget(dialog);
    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    const int mode = dialog->exec();

    if (mode == DDialog::Accepted) {
        if (pwdCheckbox->isChecked() && pwdLineEdit->text().size() > 0) {
            m_isUsePassword = pwdCheckbox->isChecked();
            m_password = pwdLineEdit->text();
        }
    }

    delete dialog;
}

bool AddCompressUsePasswordQuery::getIsUsePassword()
{
    return m_isUsePassword;
}

QString AddCompressUsePasswordQuery::getPassword()
{
    return m_password;
}
