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

#ifndef SINGLEFILEPAGE_H
#define SINGLEFILEPAGE_H

#include "fileViewer.h"

#include <DWidget>
#include <DFileDialog>
#include <DPushButton>
#include <DLineEdit>
#include <DLabel>
#include <DCommandLinkButton>
#include <DPalette>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

class Lib_Edit_Button;
class ArchiveSortFilterModel;
class CustomPushButton;
class CustomCommandLinkButton;

class UnCompressPage : public DWidget
{
    Q_OBJECT

public:
    UnCompressPage(QWidget *parent = nullptr);

    void setModel(ArchiveSortFilterModel *model);
    QString getDecompressPath();
    void setdefaultpath(QString path);
    void SetDefaultFile(QFileInfo info);
    int getFileCount();
    int getDeFileCount();
    int showWarningDialog(const QString &msg);
    EXTRACT_TYPE getExtractType();
    void setRootPathIndex();

    /**
     * @brief setWidth      设置界面宽度
     * @param windowWidth   宽度
     */
    void setWidth(int windowWidth);
    void slotCompressedAddFile();
    fileViewer *getFileViewer();
    CustomPushButton *getNextbutton();
    CustomCommandLinkButton *getPathCommandLinkButton();
    int showReplaceDialog(QString name, int &responseValue);
    void resizeEvent(QResizeEvent *event) override;

    // 设置更新文件
    void setUpdateFiles(const QStringList &listFiles);

    //提示是否转换压缩文件格式
    QStringList convertArchiveDialog();

    /**
     * @brief addPasseord   获取追加输入的密码
     * @return
     */
    QString getAddPasseord();

signals:
    void sigDecompressPress(const QString &localPath, QString conVert = "");
    void sigextractfiles(QVector<Archive::Entry *>, QString path, EXTRACT_TYPE type);
    void sigOpenExtractFile(const QVector<Archive::Entry *> &fileList, const QString &programma);
    void sigFilelistIsEmpty();
//    void sigRefreshFileList(const QStringList &files);
    /**
     * @brief sigRefreshEntryVector
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     */
    void sigRefreshEntryVector(QVector<Archive::Entry *> &vectorDel, bool isManual);
    void sigAutoCompress(const QString &, const QStringList &);
    void sigAutoCompressEntry(const QString &, const QStringList &, Archive::Entry *pWorkEntry); //added by hsw 20200525
    void sigUpdateUnCompreeTableView(const QFileInfo &);
    void sigSelectedFiles(QStringList &files);
    void subWindowTipsPopSig(int, const QStringList &);
    void subWindowTipsUpdateEntry(int, QVector<Archive::Entry *> &vectorDel);
    void sigDeleteArchiveFiles(const QStringList &files, const QString &);
    void sigAddArchiveFiles(const QStringList &files, const QString &);//废弃，added by hsw 20200528

    void sigDeleteJobFinished(Archive::Entry *pWorkEntry);

public slots:
    void oneCompressPress();
    void onPathButoonClicked();
    void onextractfilesSlot(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path);
//    void onRefreshFilelist(const QStringList &filelist);
    /**
     * @brief onRefreshEntryList
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     */
    void onRefreshEntryList(QVector<Archive::Entry *> &vectorDel, bool isManual);
    void onextractfilesOpenSlot(const QVector<Archive::Entry *> &fileList, const QString &programma);
    void onAutoCompress(const QStringList &path, Archive::Entry *pWorkEntry);
    void slotSubWindowTipsPopSig(int, const QStringList &);

    void slotDeleteJobFinished(Archive::Entry *pWorkEntry);
    //转换格式
    void convertArchive();

private:
    QString getAndDisplayPath(QString path);

    /**
     * @brief handleAddFiles   处理追加压缩文件
     * @param listPath          待追加的文件
     * @return
     */
    void handleAddFiles(const QStringList &listPath);

    /**
     * @brief showEncryptionDialog  显示加密选项对话框
     * @return  是否接受操作
     */
    int showEncryptionDialog();
private:
    //文件列表
    fileViewer *m_fileviewer;
    //解压按钮
    CustomPushButton *m_nextbutton;
    QVector<Archive::Entry *> m_vectorDel;
    //选择解压路径按钮
    CustomCommandLinkButton *m_extractpath;
    QString m_pathstr;
    QFileInfo m_info;

    ArchiveSortFilterModel *m_model;
    EXTRACT_TYPE extractType = EXTRACT_TO;
    //界面宽度
    int m_iWidth;
    //存放追加、修改、删除的文件
    QStringList m_inputlist;
    QString m_strAddPassword;   // 追加压缩时添加的密码（只针对zip有效）
};
#endif
