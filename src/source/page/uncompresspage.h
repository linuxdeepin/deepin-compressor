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

#ifndef UNCOMPRESSPAGE_H
#define UNCOMPRESSPAGE_H

#include "commonstruct.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class UnCompressView;
class CustomCommandLinkButton;
class CustomPushButton;
struct UnCompressParameter;

// 解压界面（列表）
class UnCompressPage : public DWidget
{
    Q_OBJECT
public:
    explicit UnCompressPage(QWidget *parent = nullptr);
    ~UnCompressPage() override;

    /**
     * @brief setArchiveFullPath    设置压缩包全路径
     * @param strArchiveFullPath    压缩包全路径
     * @param unCompressPar         压缩参数
     */
    void setArchiveFullPath(const QString &strArchiveFullPath, UnCompressParameter &unCompressPar);

    /**
     * @brief archiveFullPath   获取压缩包全路径
     * @return
     */
    QString archiveFullPath();

    /**
     * @brief setDefaultUncompressPath  设置默认解压路径
     * @param strPath   解压路径
     */
    void setDefaultUncompressPath(const QString &strPath);

    /**
     * @brief refreshArchiveData    加载结束刷新数据
     */
    void refreshArchiveData();
    /**
     * @brief refreshDataByCurrentPathChanged 刷新删除后的显示数据
     */
    void refreshDataByCurrentPathChanged();

    /**
     * @brief addNewFiles   追加新文件
     * @param listFiles     新文件
     */
    void addNewFiles(const QStringList &listFiles);

    /**
     * @brief getCurPath    获取当前层级路径
     * @return
     */
    QString getCurPath();

//    /**
//     * @brief isModifiable     获取压缩包数据是否可以更改
//     * @return    是否可更改
//     */
//    bool isModifiable();

    /**
     * @brief clear 清空数据
     */
    void clear();

    UnCompressView *getUnCompressView() const;

    CustomPushButton *getUnCompressBtn() const;

    CustomCommandLinkButton *getUncompressPathBtn() const;

protected:
    /**
     * @brief resizeEvent   刷新解压路径显示
     * @param e
     */
    void resizeEvent(QResizeEvent *e) override;

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief elidedExtractPath     对解压路径字符串进行截取（防止太长）
     * @param strPath               解压路径
     * @return
     */
    QString elidedExtractPath(const QString &strPath);

Q_SIGNALS:
    /**
     * @brief signalUncompress  点击解压按钮
     * @param strTargetPath     解压目标路径
     */
    void signalUncompress(const QString &strTargetPath);

    /**
     * @brief signalExtract2Path    提取压缩包中文件
     * @param listSelEntry          当前选中的文件
     * @param stOptions             提取参数
     */
    void signalExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief signalDelFiels    删除压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        删除文件总大小
     */
    void signalDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize);

    /**
     * @brief signalOpenFile    打开压缩包中文件
     * @param entry             待打开的文件
     * @param strProgram        应用程序名（为空时，用默认应用程序打开）
     */
    void signalOpenFile(const FileEntry &entry, const QString &strProgram = "");

    /**
     * @brief signalAddFiles2Archive    向压缩包中添加文件
     * @param listFiles                 待添加的文件
     * @param strPassword               密码
     */
    void signalAddFiles2Archive(const QStringList &listFiles, const QString &strPassword);

private Q_SLOTS:
    /**
     * @brief slotUncompressClicked 解压按钮点击槽函数
     */
    void slotUncompressClicked();

    /**
     * @brief slotUnCompressPathClicked     设置解压路径
     */
    void slotUnCompressPathClicked();

private:
    UnCompressView *m_pUnCompressView;    // 压缩列表
    CustomCommandLinkButton *m_pUncompressPathBtn; //解压目标路径按钮
    CustomPushButton *m_pUnCompressBtn;   //解压按钮

    QString m_strArchiveFullPath;       // 压缩包名称
    QString m_strUnCompressPath;    // 解压路径
};

#endif // UNCOMPRESSPAGE_H
