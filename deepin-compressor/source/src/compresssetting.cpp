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
#include "compresssetting.h"

#include <QDebug>
#include <DFileDialog>
#include <QFormLayout>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QFileInfo>
#include <qevent.h>
#include <DDialog>
#include <DStyle>
#include <QScrollArea>
#include "DFontSizeManager"
#include "DApplicationHelper"
#include "utils.h"

TypeLabel::TypeLabel(QWidget *parent)
    : DLabel(parent)
{

}

void TypeLabel::mousePressEvent(QMouseEvent *event)
{
    emit labelClickEvent(event);
    DLabel::mousePressEvent(event);
}

CompressSetting::CompressSetting(QWidget *parent)
    : DWidget(parent)
{
}

void CompressSetting::InitUI()
{
    //DWidget *leftwidget = new DWidget(this);
    QHBoxLayout *typelayout = new QHBoxLayout;
    QHBoxLayout *layout = new QHBoxLayout;
    m_pixmaplabel = new DLabel(this);

    m_compresstype = new TypeLabel(this);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(m_compresstype);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    m_compresstype->setMinimumHeight(25);

    DStyle style;
    QPixmap pixmap = style.standardIcon(DStyle::StandardPixmap::SP_ReduceElement).pixmap(QSize(10, 10));

    typepixmap = new TypeLabel(this);
    typepixmap->setMinimumHeight(25);
    typepixmap->setPixmap(pixmap);
    m_compresstype->setText("zip");
    m_compresstype->setWordWrap(true);
    m_compresstype->setPalette(pa);
    m_compresstype->setAlignment(Qt::AlignCenter);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
//    font.setWeight(QFont::DemiBold);
//    m_compresstype->setFont(font);
    DFontSizeManager::instance()->bind(m_compresstype, DFontSizeManager::T5, QFont::DemiBold);
    typelayout->addStretch();
    typelayout->addWidget(m_compresstype, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    typelayout->addWidget(typepixmap, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    typelayout->addStretch();

    m_clicklabel = new TypeLabel(this);
    m_clicklabel->setMinimumSize(125,40);
    m_clicklabel->setLayout(typelayout);
    layout->addWidget(m_clicklabel,0, Qt::AlignHCenter | Qt::AlignVCenter);

    m_typemenu = new DMenu(this);
    m_typemenu->setMinimumWidth(162);
    for (const QString &type : qAsConst(m_supportedMimeTypes)) {
        m_typemenu->addAction(QMimeDatabase().mimeTypeForName(type).preferredSuffix());
    }
    setTypeImage("zip");

    QFormLayout *filelayout = new QFormLayout();
    m_filename = new DLineEdit(this);
    m_filename->setMinimumSize(260, 36);
    m_filename->setText(tr("Create New Archive"));
    QLineEdit *qfilename = m_filename->lineEdit();
    qfilename->setMaxLength(70);

    m_savepath = new DFileChooserEdit(this);
    m_savepath->setFileMode(DFileDialog::Directory);
    m_savepath->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_savepath->setMinimumSize(260, 36);

    filelayout->addRow(tr("Name") + ":", m_filename);
    filelayout->addRow(tr("Save to") + ":", m_savepath);
    filelayout->setLabelAlignment(Qt::AlignLeft);
    filelayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    filelayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    DLabel *moresetlabel = new DLabel(tr("Advanced Options"), this);
    moresetlabel->setForegroundRole(DPalette::WindowText);
    m_moresetbutton = new DSwitchButton(this);
    m_moresetlayout = new QHBoxLayout();
    m_moresetlayout->addWidget(moresetlabel, 0, Qt::AlignLeft);
    m_moresetlayout->addWidget(m_moresetbutton, 1, Qt::AlignRight);
    m_encryptedlabel = new DLabel(tr("Encrypt the archive") + ":", this);
    m_encryptedlabel->setToolTip( tr("Support zip, 7z type Only"));

    m_encryptedlabel->setForegroundRole(DPalette::WindowText);
    m_password = new DPasswordEdit(this);
    QLineEdit *edit = m_password->lineEdit();
    edit->setPlaceholderText(tr("Password"));
    m_password->setMinimumSize(260, 36);
    m_encryptedfilelistlabel = new DLabel(tr("Encrypt the file list too"), this);
    m_encryptedfilelistlabel->setToolTip( tr("Support 7z type Only") );
    m_encryptedfilelistlabel->setEnabled(false);
    m_file_secret = new DSwitchButton(this);
    m_file_secretlayout = new QHBoxLayout();
    m_file_secretlayout->addWidget(m_encryptedfilelistlabel, 0, Qt::AlignLeft);
    m_file_secretlayout->addWidget(m_file_secret, 1, Qt::AlignRight);
    m_splitcompress = new DCheckBox(tr("Split to volumes") + ":", this);
    m_splitcompress->setEnabled(false);
    m_splitcompress->setToolTip( tr("Support 7z type Only") );
    m_splitnumedit = new DDoubleSpinBox(this);
    m_splitnumedit->setMinimumSize(260, 36);
    m_splitnumedit->setSuffix("MB");
    m_splitnumedit->setRange(0.0, 1000000);
    m_splitnumedit->setSingleStep(0.1);
    m_splitnumedit->setDecimals(1);
    m_splitnumedit->setValue(0.0);
    m_splitnumedit->setSpecialValueText(" ");

    QVBoxLayout *typeLayout = new QVBoxLayout;
    typeLayout->addSpacing(65);
    typeLayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    typeLayout->addLayout(layout);
    typeLayout->addStretch();
    //leftwidget->setLayout(typeLayout);

    m_fileLayout = new QVBoxLayout;
    m_fileLayout->addLayout(filelayout);
    m_fileLayout->addLayout(m_moresetlayout);
    m_fileLayout->addWidget(m_encryptedlabel);
    m_fileLayout->addWidget(m_password);
    m_fileLayout->addLayout(m_file_secretlayout);
    m_fileLayout->addWidget(m_splitcompress);
    m_fileLayout->addWidget(m_splitnumedit);
    m_fileLayout->addStretch();

    DWidget *m_rightwidget = new DWidget(this);
    m_rightwidget->setLayout(m_fileLayout);

    QScrollArea *m_scroll = new QScrollArea(this);
    m_scroll->setWidget(m_rightwidget);
    //m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setWidgetResizable(true);
    m_scroll->setMinimumHeight(345);

    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->addStretch();
    infoLayout->addLayout(typeLayout, 8);
    infoLayout->addStretch();
    infoLayout->addWidget(m_scroll, 10);
    infoLayout->addStretch();
    infoLayout->setContentsMargins(0, 0, 50, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();

    m_nextbutton = new DPushButton(tr("Compress"), this);
    m_nextbutton->setMinimumSize(340, 36);
    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_nextbutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainLayout->addLayout(buttonHBoxLayout);

    mainLayout->setContentsMargins(12, 6, 20, 20);


    m_splitnumedit->setEnabled(false);
    m_password->setEnabled(true);
    m_file_secret->setEnabled(false);

    setBackgroundRole(DPalette::Base);
}

void CompressSetting::InitConnection()
{
    connect(m_nextbutton, &DPushButton::clicked, this, &CompressSetting::onNextButoonClicked);
    connect(m_moresetbutton, &DSwitchButton::toggled, this, &CompressSetting::onAdvanceButtonClicked);
    connect(m_splitnumedit, SIGNAL(valueChanged(double)), this, SLOT(onSplitValueChanged(double)));
    connect(m_splitcompress, &DCheckBox::stateChanged, this, &CompressSetting::onSplitChanged);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &CompressSetting::onThemeChanged );
    connect(m_savepath, &DFileChooserEdit::textChanged, this, &CompressSetting::onThemeChanged );

    connect(m_filename, &DLineEdit::textChanged, this, [ = ] {
        DPalette plt;
        plt = DApplicationHelper::instance()->palette(m_filename);

        if ( false == checkfilename(m_filename->text()) )
        {
            plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));

        } else
        {
            plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
        }

        m_filename->setPalette(plt);
    });

    connect(typepixmap, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(showRightMenu(QMouseEvent *)));
    connect(m_compresstype, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(showRightMenu(QMouseEvent *)));
    connect(m_clicklabel, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(showRightMenu(QMouseEvent *)));
    connect(m_typemenu, &DMenu::triggered, this, &CompressSetting::ontypeChanged);
}

void CompressSetting::showRightMenu(QMouseEvent */*e*/)
{
    QPoint pos;
    pos.setX(window()->x() + m_pixmaplabel->x() + 60);
    pos.setY(window()->y() + m_pixmaplabel->y() + 240);
    m_typemenu->popup(pos);

}


void CompressSetting::onNextButoonClicked()
{
    QDir dir(m_savepath->text());
    QString name = m_filename->text().remove(" ");
    if (!checkfilename(name)) {
        DDialog *dialog = new DDialog(this);
        QPixmap pixmap = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(30, 30));
        dialog->setIcon(pixmap);
        dialog->setMessage(tr("The file name is error, please retry"));
        dialog->addSpacing(15);
        dialog->addButton(tr("OK"));
        dialog->exec();
        delete dialog;
        return;
    } else if (m_savepath->text().remove(" ") == "") {
        DDialog *dialog = new DDialog(this);
        QPixmap pixmap = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(30, 30));
        dialog->setIcon(pixmap);
        dialog->setMessage(tr("Please enter the path"));
        dialog->addSpacing(15);
        dialog->addButton(tr("OK"));
        dialog->exec();
        delete dialog;
        return;
    } else if (!dir.exists()) {
        DDialog *dialog = new DDialog(this);
        QPixmap pixmap = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(30, 30));
        dialog->setIcon(pixmap);
        dialog->setMessage(tr("The path does not exist, please retry"));
        dialog->addSpacing(15);
        dialog->addButton(tr("OK"));
        dialog->exec();
        delete dialog;
        return;
    }
    QMap<QString, QString> m_openArgs;
    const QString password = m_password->text();
    QString fixedMimeType;
    for (const QString &type : qAsConst(m_supportedMimeTypes)) {
//        QString txt = m_compresstype->text();
//        QString preferredSuf = QMimeDatabase().mimeTypeForName(type).preferredSuffix();
        if ( 0 == QMimeDatabase().mimeTypeForName(type).preferredSuffix().compare(m_compresstype->text(), Qt::CaseSensitive) ) {
            fixedMimeType = type;
            break;
        }
    }

    m_openArgs[QStringLiteral("createNewArchive")] = QStringLiteral("true");
    m_openArgs[QStringLiteral("fixedMimeType")] = fixedMimeType;
    if ("application/x-tar" == fixedMimeType || "application/x-tarz" == fixedMimeType) {
        m_openArgs[QStringLiteral("compressionLevel")] = "-1";//-1 is unuseful
    } else {
        m_openArgs[QStringLiteral("compressionLevel")] = "6";//6 is default
    }

    qDebug() << m_splitnumedit->value();
    if (m_splitnumedit->value() > 0) {
        m_openArgs[QStringLiteral("volumeSize")] = QString::number(static_cast<int>(m_splitnumedit->value() * 1024));
    }
//    if (!dialog.data()->compressionMethod().isEmpty()) {
//        m_openArgs.metaData()[QStringLiteral("compressionMethod")] = dialog.data()->compressionMethod();
//    }
    qDebug() << m_openArgs[QStringLiteral("volumeSize")];
    if (!m_password->text().isEmpty()) {
        m_openArgs[QStringLiteral("encryptionMethod")] = "AES256";//5 is default
    }

    m_openArgs[QStringLiteral("encryptionPassword")] = password;

    if (m_file_secret->isChecked()) {
        m_openArgs[QStringLiteral("encryptHeader")] = QStringLiteral("true");
    }

    m_openArgs[QStringLiteral("localFilePath")] = m_savepath->text();
    m_openArgs[QStringLiteral("filename")] = m_filename->text() + "." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();


    emit sigCompressPressed(m_openArgs);


    m_openArgs.remove(QStringLiteral("createNewArchive"));
    m_openArgs.remove(QStringLiteral("fixedMimeType"));
    m_openArgs.remove(QStringLiteral("compressionLevel"));
    m_openArgs.remove(QStringLiteral("encryptionPassword"));
    m_openArgs.remove(QStringLiteral("encryptHeader"));
    m_openArgs.remove(QStringLiteral("localFilePath"));
    m_openArgs.remove(QStringLiteral("filename"));
}

void CompressSetting::onAdvanceButtonClicked(bool status)
{
    if (status) {
        m_encryptedlabel->setVisible(true);
        m_password->setVisible(true);
        m_splitcompress->setVisible(true);
        m_encryptedfilelistlabel->setVisible(true);
        m_file_secret->setVisible(true);
        m_splitcompress->setVisible(true);
        m_splitnumedit->setVisible(true);
    } else {
        m_encryptedlabel->setVisible(false);
        m_password->setVisible(false);
        m_splitcompress->setVisible(false);
        m_encryptedfilelistlabel->setVisible(false);
        m_file_secret->setVisible(false);
        m_splitcompress->setVisible(false);
        m_splitnumedit->setVisible(false);
    }
}

void CompressSetting::setTypeImage(QString type)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp."+type));

    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}

void CompressSetting::setDefaultPath(QString path)
{
    if(m_nextbutton == nullptr)
    {
        m_supportedMimeTypes = m_pluginManger.supportedWriteMimeTypes(PluginManager::SortByComment);
        InitUI();
        InitConnection();
    }

    m_savepath->setText(path);
    QUrl dir(path);
    m_savepath->setDirectoryUrl(dir);
}

void CompressSetting::setDefaultName(QString name)
{
    if(m_nextbutton == nullptr)
    {
        m_supportedMimeTypes = m_pluginManger.supportedWriteMimeTypes(PluginManager::SortByComment);
        InitUI();
        InitConnection();
    }

    onAdvanceButtonClicked(m_moresetbutton->isChecked());
    m_filename->setText(name);
    QLineEdit *qfilename = m_filename->lineEdit();
    qfilename->selectAll();
    qfilename->setFocus();
}

quint64 CompressSetting::dirFileSize(const QString &path)
{
    QDir dir(path);
    quint64 size = 0;
    //dir.entryInfoList(QDir::Files)返回文件信息
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files)) {
        //计算文件大小
        size += fileInfo.size();
    }
    //dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)返回所有子目录，并进行过滤
    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        //若存在子目录，则递归调用dirFileSize()函数
        size += dirFileSize(path + QDir::separator() + subDir);
    }
    return size;
}

bool CompressSetting::checkfilename(QString str)
{
    if (str.length() == 0) {
        return false;
    }
    if (str.length() > 255) {
        return false;
    }

    QList<QChar> black_list;
    black_list << '+';
    black_list << '-';
    black_list << '.';
    black_list << ',';
    if (black_list.contains(str.at(0))) {
        return false;
    }

    black_list.clear();
    black_list = {'/', '\t', '\b', '@', '#', '$', '%', '^', '&'};/*, '*', '(', ')', '[', ']'*/
    foreach (QChar black_char, black_list) {
        if (str.contains(black_char)) {
            return false;
        }
    }

    return true;
}

void CompressSetting::showEvent(QShowEvent *event)
{
    if(m_nextbutton == nullptr)
    {
        m_supportedMimeTypes = m_pluginManger.supportedWriteMimeTypes(PluginManager::SortByComment);
        InitUI();
        InitConnection();
    }

    DWidget::showEvent(event);
}


void CompressSetting::keyPressEvent(QKeyEvent *event)
{
    if (!event) {
        return;
    }
    if (event->key() == Qt::Key_F2) {

        QLineEdit *qfilename = m_filename->lineEdit();
        QLineEdit *qpath = m_savepath->lineEdit();
        QLineEdit *qpassword = m_password->lineEdit();
        if (qfilename->hasFocus()) {
            qfilename->selectAll();
        } else if (qpath->hasFocus()) {
            qpath->selectAll();
        } else if (qpassword->hasFocus()) {
            qpassword->selectAll();
        } else if (m_splitnumedit->hasFocus()) {
            m_splitnumedit->selectAll();
        }
    }
}

void CompressSetting::setFilepath(QStringList pathlist)
{
    m_pathlist = pathlist;
}

void CompressSetting::onSplitChanged(int /*status*/)
{
    if (m_splitcompress->isChecked() && "7z" == m_compresstype->text()) {
        m_splitnumedit->setEnabled(true);
    } else {
        m_splitnumedit->setEnabled(false);
    }
}

void CompressSetting::ontypeChanged(QAction *action)
{
    qDebug() << action->text();

    setTypeImage(action->text());
    m_compresstype->setText(action->text());

    if (action->text().contains("7z")) {
        if (m_splitcompress->isChecked()) {
            m_splitnumedit->setEnabled(true);
        }
        m_encryptedlabel->setEnabled(true);
        m_password->setEnabled(true);
        m_encryptedfilelistlabel->setEnabled(true);
        m_file_secret->setEnabled(true);
        m_splitcompress->setEnabled(true);
    } else if (action->text().contains("zip")) {
        m_splitnumedit->setEnabled(false);
        m_encryptedlabel->setEnabled(true);
        m_password->setEnabled(true);
        m_encryptedfilelistlabel->setEnabled(false);
        m_file_secret->setEnabled(false);
        m_file_secret->setChecked(false);
        m_splitcompress->setEnabled(false);
        m_splitcompress->setChecked(false);
        m_splitnumedit->setRange(0.0, 5.0);
        m_splitnumedit->setValue(0.0);
    } else {
        m_splitnumedit->setEnabled(false);
        m_encryptedlabel->setEnabled(false);
        m_password->setEnabled(false);
        m_encryptedfilelistlabel->setEnabled(false);
        m_file_secret->setEnabled(false);
        m_file_secret->setChecked(false);
        m_splitcompress->setEnabled(false);
        m_splitcompress->setChecked(false);
        m_splitnumedit->setRange(0.0, 5.0);
        m_splitnumedit->setValue(0.0);
    }
}

void CompressSetting::onSplitValueChanged(double /*value*/)
{
//    int filesize = 0;
//    foreach(QString path, m_pathlist)
//    {
//        filesize += dirFileSize(path);
//    }

//    if(filesize < 1024*1024*5)
//    {
//        m_splitnumedit->setEnabled(false);
//        m_splitnumedit->setValue(double(filesize)/(1024*1024));
//    }
//    else if(value < 5.0){
//        m_splitnumedit->setValue(5.0);
//        m_splitnumedit->setRange(5.0, double(filesize)/(1024*1024));
//    }
}


void CompressSetting::onRetrunPressed()
{

}

void CompressSetting::onThemeChanged()
{
    DPalette plt;
    plt = DApplicationHelper::instance()->palette(m_filename);

    if ( false == checkfilename(m_filename->text()) )
    {
        plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));

    } else
    {
        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
    }

    m_filename->setPalette(plt);
}
