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

#ifndef COMPRESSPAGE_H
#define COMPRESSPAGE_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class CompressView;
class CustomPushButton;

// 压缩界面（列表）
class CompressPage : public DWidget
{
    Q_OBJECT
public:
    explicit CompressPage(QWidget *parent = nullptr);
    ~CompressPage() override;

    /**
     * @brief addCompressFiles  添加压缩文件
     * @param listFiles     添加的文件
     */
    void addCompressFiles(const QStringList &listFiles);

    /**
     * @brief compressFiles     获取所有待压缩的文件
     * @return 所有待压缩的文件
     */
    QStringList compressFiles();

    /**
     * @brief refreshCompressedFiles    刷新压缩文件
     * @param bChanged                  是否有文件改变
     * @param strFileName               改变的文件名
     */
    void refreshCompressedFiles(bool bChanged = false, const QString &strFileName = "");

    /**
     * @brief clear 清空数据
     */
    void clear();

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

signals:
    /**
     * @brief signalFileChoose  选择文件信号
     */
    void signalFileChoose();

    /**
     * @brief signalCompressNext    压缩下一步信号
     */
    void signalCompressNextClicked();

    /**
     * @brief signalLevelChanged    目录层级变化
     * @param bRootIndex    是否是根目录
     */
    void signalLevelChanged(bool bRootIndex);

private slots:
    /**
     * @brief slotCompressNext  按钮点击下一步
     */
    void slotCompressNextClicked();

    /**
     * @brief compressLevelChanged  处理压缩层级变化
     * @param bRootIndex    是否是根目录
     */
    void slotCompressLevelChanged(bool bRootIndex);

private:
    CompressView *m_pCompressView;    // 压缩列表
    CustomPushButton *m_pNextBtn;       // 下一步
};

#endif // COMPRESSPAGE_H
