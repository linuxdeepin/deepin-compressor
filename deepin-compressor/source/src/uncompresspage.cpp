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

#include "uncompresspage.h"
#include "utils.h"
#include "archivemodel.h"
#include "lib_edit_button.h"
#include "archivesortfiltermodel.h"
#include "queries.h"
#include "customwidget.h"
#include "DebugTimeManager.h"

#include <DStandardPaths>
#include <DMessageManager>
#include <DDialog>
#include <DFontSizeManager>
#include <DRadioButton>

#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QFontMetrics>
#include <QMessageBox>
#include <QUuid>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

UnCompressPage::UnCompressPage(QWidget *parent)
    : DWidget(parent)
{
    m_pathstr = "~/Desktop";
    m_fileviewer = new fileViewer(this, PAGE_UNCOMPRESS);
    m_nextbutton = new CustomPushButton(tr("Extract"), this);
    m_nextbutton->setAccessibleName("Extract_btn");
    m_nextbutton->setMinimumSize(340, 36);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    // 解压路径
    m_extractpath = new CustomCommandLinkButton(tr("Extract to:") + " ~/Desktop", this);
    m_extractpath->setAccessibleName("Extractpath_btn");
    m_extractpath->setToolTip(m_pathstr);
//    m_extractpath->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
//    m_extractpath->setMinimumSize(129, 18);
    DFontSizeManager::instance()->bind(m_extractpath, DFontSizeManager::T8);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(m_nextbutton, 2);
    buttonlayout->addStretch(1);

    QHBoxLayout *pathlayout = new QHBoxLayout;
    pathlayout->addStretch(1);
    pathlayout->addWidget(m_extractpath, 2, Qt::AlignCenter);
    pathlayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(pathlayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 9);
    mainLayout->setStretchFactor(pathlayout, 1);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(12, 1, 20, 20);

    setBackgroundRole(DPalette::Base);

    connect(m_nextbutton, &DPushButton::clicked, this, &UnCompressPage::oneCompressPress);
    connect(m_extractpath, &DPushButton::clicked, this, &UnCompressPage::onPathButoonClicked);
    connect(m_fileviewer, &fileViewer::sigextractfiles, this, &UnCompressPage::onextractfilesSlot);
    connect(m_fileviewer, &fileViewer::sigOpenWith, this, &UnCompressPage::onextractfilesOpenSlot);
//    connect(m_fileviewer, &fileViewer::sigFileRemoved, this, &UnCompressPage::onRefreshFilelist);
    connect(m_fileviewer, &fileViewer::sigNeedConvert, this, &UnCompressPage::convertArchive);
    connect(m_fileviewer, &fileViewer::sigEntryRemoved, this, &UnCompressPage::onRefreshEntryList);
    connect(m_fileviewer, &fileViewer::sigFileAutoCompress, this, &UnCompressPage::onAutoCompress);
    connect(this, &UnCompressPage::subWindowTipsPopSig, m_fileviewer, &fileViewer::SubWindowDragMsgReceive);
//    connect(this, &UnCompressPage::subWindowTipsUpdateEntry, m_fileviewer, &fileViewer::SubWindowDragUpdateEntry);
    connect(m_fileviewer, &fileViewer::sigFileRemovedFromArchive, this, &UnCompressPage::sigDeleteArchiveFiles);

    setTabOrder(m_extractpath, m_nextbutton);
//    connect(m_fileviewer, &fileViewer::sigFileAutoCompressToArchive, this, &UnCompressPage::sigAddArchiveFiles);
}

/**
 * @brief UnCompressPage::oneCompressPress 点击解压按钮
 */
void UnCompressPage::oneCompressPress()
{
    PERF_PRINT_BEGIN("POINT-04", "解压名:" + m_info.fileName() + "  解压大小:" + QString::number(m_info.size()) + "B"); //解压计时
    QFileInfo m_fileDestinationPath(m_pathstr);
    bool m_permission = (m_fileDestinationPath.isWritable() && m_fileDestinationPath.isExecutable());

    if (!m_permission) { // 无法解压到已选中路径
        if (!m_fileDestinationPath.exists()) { // 路径不存在
            showWarningDialog(tr("The default extraction path does not exist, please retry"));
        } else { // 路径无权限
            showWarningDialog(tr("You do not have permission to save files here, please change and retry"));
        }
        return;
    } else { // 发送解压信号
        emit sigDecompressPress(m_pathstr);
    }
}

/**
 * @brief UnCompressPage::setModel 设置数据模型
 * @param model
 */
void UnCompressPage::setModel(ArchiveSortFilterModel *model)
{
    m_model = model;
    m_fileviewer->setDecompressModel(m_model);
}

/**
 * @brief UnCompressPage::onPathButoonClicked 选择路径
 */
void UnCompressPage::onPathButoonClicked()
{
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setWindowTitle(tr("Find directory"));
    dialog.setDirectory(m_pathstr);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    QList<QUrl> pathlist = dialog.selectedUrls();
    QString str = pathlist.at(0).toLocalFile();
    m_pathstr = str;
    m_extractpath->setToolTip(m_pathstr);

    str = getAndDisplayPath(str);
    m_extractpath->setText(tr("Extract to:") + str);
}

/**
 * @brief UnCompressPage::setdefaultpath 设置解压路径
 * @param path
 */
void UnCompressPage::setdefaultpath(const QString path)
{
    m_pathstr = path;
    m_extractpath->setToolTip(m_pathstr);

    QString str = path;
    str = getAndDisplayPath(str);

    m_extractpath->setText(tr("Extract to:") + str);
}

/**
 * @brief UnCompressPage::SetDefaultFile 设置解压文件信息
 * @param info
 */
void UnCompressPage::SetDefaultFile(QFileInfo info)
{
    m_info = info;
}

/**
 * @brief UnCompressPage::getFileCount 获取文件数量
 * @return
 */
int UnCompressPage::getFileCount()
{
    return m_fileviewer->getFileCount();
}

//获取被解压文件里一级文件(夹)个数
int UnCompressPage::getDeFileCount()
{
    return m_fileviewer->getDeFileCount();
}

int UnCompressPage::showWarningDialog(const QString &msg)
{
    DDialog *dialog = new DDialog(this);
    dialog->setAccessibleName("Warning_dialog");
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));

    dialog->setIcon(pixmap);
    dialog->addButton(tr("OK"));
    dialog->setMinimumSize(380, 140);

    DLabel *pContent = new DLabel(msg, dialog);
    pContent->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(pContent);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ButtonText));
    DFontSizeManager::instance()->bind(pContent, DFontSizeManager::T6, QFont::Medium);
//    pContent->setMinimumWidth(this->width());
//    pContent->move(dialog->width() / 2 - pContent->width() / 2, dialog->height() / 2 - pContent->height() / 2 - 10);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(pContent, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget *widget = new DWidget(dialog);
    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    int res = dialog->exec();
    delete dialog;

    return res;
}

/**
 * @brief UnCompressPage::getExtractType 获取解压类型
 * @return
 */
EXTRACT_TYPE UnCompressPage::getExtractType()
{
    return extractType;
}

void UnCompressPage::setRootPathIndex()
{
    m_fileviewer->setRootPathIndex();
}

/**
 * @brief UnCompressPage::setWidth 获取应用窗口宽度
 * @param windowWidth
 */
void UnCompressPage::setWidth(int windowWidth)
{
    m_iWidth = windowWidth;
}

/**
 * @brief UnCompressPage::resizeEvent 窗口大小发生变化
 * @param event
 */
void UnCompressPage::resizeEvent(QResizeEvent *event)
{
    setWidth(width());
    setdefaultpath(m_pathstr);
    QWidget::resizeEvent(event);
}

/**
 * @brief UnCompressPage::setUpdateFiles 更新文件列表
 * @param listFiles
 */
void UnCompressPage::setUpdateFiles(const QStringList &listFiles)
{
    m_inputlist = listFiles;
}

/**
 * @brief UnCompressPage::convertArchive 格式转换
 */
void UnCompressPage::convertArchive()
{
    QStringList type = convertArchiveDialog(); //  确认是否进行格式转换
    if (type.at(0) == "true") {
        // 格式转换，需要将压缩文件解压到临时路径
        QString tmppath = TEMPDIR_NAME + PATH_SEP + "converttempfiles" + PATH_SEP + Utils::createRandomString();
        //QString tmppath = TEMPDIR_NAME + PATH_SEP + "converttempfiles";
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkpath(tmppath);
        }

        if (type.last() == "zip") { // 转换为zip格式
            emit sigDecompressPress(tmppath, "zip"); // 发送解压信号
        } else if (type.last() == "7z") { // 转换为7z格式
            emit sigDecompressPress(tmppath, "7z");
        }
    }

//    if (m_info.filePath().endsWith(".rar")) {
//        QStringList type = convertArchiveDialog();
//        if (type.at(0) == "true") {
//            QString strTemp = Utils::createRandomString();
//            QString tmppath = TEMPDIR_NAME + PATH_SEP + "converttempfiles" + PATH_SEP + strTemp /*Utils::createRandomString()*/;
//            //QString tmppath = TEMPDIR_NAME + PATH_SEP + "converttempfiles";
//            QDir dir(tmppath);
//            if (!dir.exists()) {
//                dir.mkpath(tmppath);
//            }
//            //            QString tmppath1 = TEMPDIR_NAME;
//            //            QDir dir1(tmppath1);
//            //            if (!dir1.exists()) {
//            //                dir1.mkdir(tmppath1);
//            //            }

//            //            QString tmppath2 = TEMPDIR_NAME + PATH_SEP /*+ Utils::createRandomString() + PATH_SEP*/ + "converttempfiles";
//            //            QDir dir2(tmppath2);
//            //            if (!dir2.exists()) {
//            //                dir1.mkpath(tmppath2);
//            //            }

//            if (type.last() == "zip") {
//                emit sigDecompressPress(tmppath, "zip");
//            } else if (type.last() == "7z") {
//                emit sigDecompressPress(tmppath, "7z");
//            }
//        }
//    } else {
//        emit sigAutoCompress(m_info.filePath(), m_inputlist);
//    }
}

/**
 * @brief UnCompressPage::getAndDisplayPath 获取大小长度合适的解压路径字符串
 * @param path
 * @return
 */
QString UnCompressPage::getAndDisplayPath(QString path)
{
    const QString curpath = path;
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(curpath);//获取之前设置的字符串的像素大小
    QString pathStr = curpath;
    if (fontSize > m_iWidth) {
        pathStr = fontMetrics.elidedText(path, Qt::ElideMiddle, m_iWidth);//返回一个带有省略号的字符串
    }

    return pathStr;
}

/**
 * @brief UnCompressPage::slotCompressedAddFile 解压列表界面添加文件进行追加压缩
 */
void UnCompressPage::slotCompressedAddFile()
{
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setAllowMixedSelection(true);

    const int mode = dialog.exec();;

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    QVector<Archive::Entry *> vectorEntry;
    m_inputlist.clear();
    ArchiveModel *pModel = dynamic_cast<ArchiveModel *>(m_model->sourceModel());
    int responseValue = 0;
    foreach (QString strPath, dialog.selectedFiles()) {
        Archive::Entry *entry = pModel->isExists(strPath);
        if (entry != nullptr) {
            int mode = showReplaceDialog(strPath, responseValue);
            if (1 == mode) {
                vectorEntry.push_back(entry);
                m_inputlist.push_back(strPath);
            }
        } else {
            m_inputlist.push_back(strPath);
        }
    }

//    m_model->refreshNow();
//    if (vectorEntry.count() > 0) {
//        emit onRefreshEntryList(vectorEntry, false);
//    } else {
//        if (m_inputlist.count() > 0)
//            emit sigAutoCompress(m_info.filePath(), m_inputlist);
//        //emit onAutoCompress(m_inputlist);

//        m_inputlist.clear();
//    }

    m_model->refreshNow();
    if (vectorEntry.count() > 0) {
        emit onRefreshEntryList(vectorEntry, false);
    } else {
        if (m_inputlist.count() > 0) {
            if (m_info.filePath().endsWith(".rar")) { // 如果是rar文件，需要进行格式转换
                convertArchive();
            } else {
                emit sigAutoCompress(m_info.filePath(), m_inputlist);
            }
        }

        //emit onAutoCompress(m_inputlist);
        m_inputlist.clear();
    }

    //emit sigAutoCompress(m_info.filePath(), dialog.selectedFiles());
}

fileViewer *UnCompressPage::getFileViewer()
{
    return m_fileviewer;
}

CustomPushButton *UnCompressPage::getNextbutton()
{
    return m_nextbutton;
}

CustomCommandLinkButton *UnCompressPage::getPathCommandLinkButton()
{
    return m_extractpath;
}

/**
 * @brief UnCompressPage::getDecompressPath 获取解压路径
 * @return
 */
QString UnCompressPage::getDecompressPath()
{
    return m_pathstr;
}

/**
 * @brief UnCompressPage::onextractfilesSlot 解压列表右键菜单对应提取的操作
 * @param fileList
 * @param type
 * @param path
 */
void UnCompressPage::onextractfilesSlot(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path)
{
    if (fileList.count() == 0) {
        return;
    }

    // get extract type
    extractType = type;

    if (EXTRACT_TO == type) {//菜单“提取”
        DFileDialog dialog(this);
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setDirectory(m_pathstr);

        const int mode = dialog.exec();

        if (mode != QDialog::Accepted) {
            return;
        }

        QList<QUrl> pathlist = dialog.selectedUrls();
        QString curpath = pathlist.at(0).toLocalFile();

        emit sigextractfiles(fileList, curpath, type);
    } else if (EXTRACT_DRAG == type) { // 拖拽提取
        emit sigextractfiles(fileList, path, type);
    } else if (EXTRACT_TEMP == type) { // 双击打开文件
        QString tmppath = TEMPDIR_NAME + PATH_SEP + Utils::createRandomString();
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkpath(tmppath);
        }

        emit sigextractfiles(fileList, tmppath, type);
    } else if (EXTRACT_TEMP_CHOOSE_OPEN == type) { // 菜单“打开”
        QString tmppath = TEMPDIR_NAME + PATH_SEP + Utils::createRandomString();
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkpath(tmppath);
        }

        emit sigextractfiles(fileList, tmppath, type);
    } else {
        emit sigextractfiles(fileList, m_pathstr, type);
    }
}

//void UnCompressPage::onRefreshFilelist(const QStringList &filelist)
//{
//    m_filelist = filelist;
//  //    m_fileviewer->setFileList(m_filelist);

//    emit sigRefreshFileList(m_filelist);

//    if (m_filelist.size() == 0) {
//        emit sigFilelistIsEmpty();
//    }
//}

void UnCompressPage::onRefreshEntryList(QVector<Archive::Entry *> &vectorDel, bool isManual)
{
    m_vectorDel = vectorDel;
//    emit sigRefreshFileList(m_filelist);
    emit sigRefreshEntryVector(m_vectorDel, isManual);
    if (m_vectorDel.size() == 0) {
        emit sigFilelistIsEmpty();
    }
}

void UnCompressPage::onextractfilesOpenSlot(const QVector<Archive::Entry *> &fileList, const QString &programma)
{
    emit sigOpenExtractFile(fileList, programma);
}

/**
 * @brief UnCompressPage::onAutoCompress 执行压缩操作
 * @param path
 * @param pWorkEntry
 */
void UnCompressPage::onAutoCompress(const QStringList &path, Archive::Entry *pWorkEntry)
{
    m_inputlist.clear();

    if (!m_fileviewer->isDropAdd()) {
        //m_inputlist = path;
        emit sigAutoCompressEntry(m_info.filePath(), path, pWorkEntry);
        return;
    }

    ArchiveModel *pModel = dynamic_cast<ArchiveModel *>(m_model->sourceModel());
    QVector<Archive::Entry *> vectorEntry;
    //int mode = 0;
    int mode = 0;
    bool bAll = false;
//    int responseValue = Result_Cancel;
    foreach (QString strPath, path) {
        Archive::Entry *entry = pModel->isExists(strPath);

        if (entry != nullptr) {
            if (!bAll) {
                OverwriteQuery query(strPath);
                query.setParent(this);
                query.execute();
                mode = query.getExecuteReturn();

                bAll = query.applyAll();
            }

            if (1 == mode) { //替换
                vectorEntry.push_back(entry);
                m_inputlist.push_back(strPath);
            }
        } else {
            m_inputlist.push_back(strPath);
        }
    }

    m_model->refreshNow();
    if (vectorEntry.count() > 0) {
        emit onRefreshEntryList(vectorEntry, false);
    } else {
        if (m_inputlist.count() > 0) {
            if (m_info.filePath().endsWith(".rar")) {
                convertArchive();
            } else {
                emit sigAutoCompress(m_info.filePath(), m_inputlist);
            }
        }

        //emit onAutoCompress(m_inputlist);
        m_inputlist.clear();
    }
}

void UnCompressPage::slotSubWindowTipsPopSig(int mode, const QStringList &args)
{
    emit subWindowTipsPopSig(mode, args);
}

/**
 * @brief UnCompressPage::slotDeleteJobFinished 删除操作结束
 * @param pWorkEntry
 */
void UnCompressPage::slotDeleteJobFinished(Archive::Entry *pWorkEntry)
{
    if (m_inputlist.count() > 0) { // 重新压缩剩余文件
//        emit sigAutoCompressEntry(m_info.filePath(), m_inputlist, pWorkEntry);
        emit sigAutoCompress(m_info.filePath(), m_inputlist);
    }

//    emit sigAutoCompress(m_info.filePath(), m_inputlist);

    m_inputlist.clear();

    emit sigDeleteJobFinished(pWorkEntry);
}

/**
 * @brief UnCompressPage::showReplaceDialog 添加重复文件提示
 * @param name
 * @param responseValue
 * @return
 */
int UnCompressPage::showReplaceDialog(QString name, int &responseValue)
{
    OverwriteQuery query(name);
    query.setParent(this);
    query.execute();
    responseValue = query.response().toInt();
    return query.getExecuteReturn();
}

/**
 * @brief UnCompressPage::convertArchiveDialog 提示是否进行格式转换
 * @return
 */
QStringList UnCompressPage::convertArchiveDialog()
{
    DDialog *dialog = new DDialog(this);
    dialog->setAccessibleName("Conver_dialog");
    dialog->setMinimumSize(QSize(380, 180));

    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));

    dialog->setIcon(pixmap);
    dialog->addButton(tr("Cancel"));
    dialog->addButton(tr("Convert"), true, DDialog::ButtonRecommend);

    DWidget *widget = new DWidget(dialog);
    QVBoxLayout *mainlayout = new QVBoxLayout(widget);
    QHBoxLayout *textLayout = new QHBoxLayout;
    QHBoxLayout *labelLayout = new QHBoxLayout;

    DLabel *strlabel = new DLabel(/*dialog*/);
    strlabel->setMinimumSize(QSize(308, 40));
    strlabel->setText(tr("Changes to archives in this file type are not supported. Please convert the archive format to save the changes."));
//    strlabel->adjustSize();
    strlabel->setWordWrap(true);
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    textLayout->setSpacing(36);
    textLayout->addWidget(strlabel/*, Qt::AlignHCenter | Qt::AlignVCenter*/);
    textLayout->setSpacing(36);

//    DPalette pa = DApplicationHelper::instance()->palette(strlabel);
//    pa.setBrush(DPalette::ToolTipText, pa.color(DPalette::ToolTipText));
//    DApplicationHelper::instance()->setPalette(strlabel, pa);

    DLabel *strlabel2 = new DLabel(/*dialog*/);
    strlabel2->setMinimumSize(QSize(112, 20));
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setText(tr("Convert the format to:"));

    DRadioButton *zipBtn = new DRadioButton("ZIP");
    zipBtn->setChecked(true);
    DRadioButton *_7zBtn = new DRadioButton("7Z");
    _7zBtn->setChecked(false);

    labelLayout->addStretch();
    labelLayout->addWidget(strlabel2);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(zipBtn);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(_7zBtn);
    labelLayout->addStretch();

    mainlayout->addWidget(strlabel);
    mainlayout->addStretch();
    mainlayout->addLayout(labelLayout);

    widget->setLayout(mainlayout);

    dialog->addContent(widget);
    dialog->setTabOrder(zipBtn, _7zBtn);
    dialog->setTabOrder(_7zBtn, dialog->getButton(0));
    dialog->setTabOrder(dialog->getButton(0), dialog->getButton(1));

    QStringList typeList;
    bool isZipConvert = true;
    bool is7zConvert = false;
    QString convertType;

    // 转换zip格式
    connect(zipBtn, &DRadioButton::toggled, this, [ =, &isZipConvert]() {
        isZipConvert = zipBtn->isChecked();
        qDebug() << "zip" << isZipConvert;
    });

    // 转换为7z格式
    connect(_7zBtn, &DRadioButton::toggled, this, [ =, &is7zConvert]() {
        is7zConvert = _7zBtn->isChecked();
        qDebug() << "7z" << is7zConvert;
    });

    qDebug() << "zip" << isZipConvert;
    qDebug() << "7z" << is7zConvert;

    const int mode = dialog->exec();

    if (mode == QDialog::Accepted) {
        if (isZipConvert) {
            typeList << "true" << "zip";
        } else if (is7zConvert) {
            typeList << "true" << "7z";
        }
    } else {
        typeList << "false" << "none";
    }

    return typeList;
}
