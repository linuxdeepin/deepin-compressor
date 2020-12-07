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

#ifndef COMPRESSSETTINGPAGE_H
#define COMPRESSSETTINGPAGE_H

#include <DLabel>
#include <DWidget>
#include <DLineEdit>
#include <dfilechooseredit.h>
#include <DPasswordEdit>
#include <DDoubleSpinBox>
#include <DTextEdit>
#include <DDialog>

DWIDGET_USE_NAMESPACE


class CustomSwitchButton;
class CustomCheckBox;
class CustomPushButton;
class PluginManager;

// 类型控件
class TypeLabel: public DLabel
{
    Q_OBJECT
public:
    explicit TypeLabel(QWidget *parent = nullptr);
    ~TypeLabel() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

signals:
    void labelClickEvent(QMouseEvent *event);

private:
    Qt::FocusReason m_reson = Qt::NoFocusReason;
};

// 压缩设置界面
class CompressSettingPage : public DWidget
{
    Q_OBJECT
public:
    explicit CompressSettingPage(QWidget *parent = nullptr);
    ~CompressSettingPage() override;

    /**
     * @brief setFileSize   设置待压缩文件和大小
     * @param listFiles 待压缩文件
     * @param qSize 文件大小
     */
    void setFileSize(const QStringList &listFiles, qint64 qSize);

    /**
     * @brief refreshMenu   刷新压缩类型菜单
     */
    void refreshMenu();

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
     * @brief setTypeImage  设置类型图片
     * @param type  压缩类型
     */
    void setTypeImage(const QString &strType);

    /**
     * @brief checkFileNameVaild    检测文件名合法性
     * @param strText   内容
     * @return  是否合法
     */
    bool checkFileNameVaild(const QString strText);

    /**
     * @brief setEncryptedEnabled   设置加密文件是否可用
     * @param bEnabled  是否可用
     */
    void setEncryptedEnabled(bool bEnabled);

    /**
     * @brief slotEchoModeChanged   密码框明暗码切换
     * @param bEchoOn   是否明码
     */
    void slotEchoModeChanged(bool bEchoOn);

    /**
     * @brief setListEncryptionEnabled  设置列表加密是否可用
     * @param bEnabled  是否可用
     */
    void setListEncryptionEnabled(bool bEnabled);

    /**
     * @brief setSplitEnabled   设置分卷是否可用
     * @param bEnabled  是否可用
     */
    void setSplitEnabled(bool bEnabled);

    /**
     * @brief refreshCompressLevel  刷新压缩方式
     * @param strType               格式类型
     */
    void refreshCompressLevel(const QString &strType);

    /**
     * @brief setCommentEnabled 设置注释是否可用
     * @param bEnabled      是否可用
     */
    void setCommentEnabled(bool bEnabled);

    /**
     * @brief checkCompressOptionValid 检查压缩设置选项是否有效
     * @return
     */
    bool checkCompressOptionValid();

    /**
     * @brief CompressSetting::checkFilePermission 检查文件权限
     * @param path
     * @return
     */
    bool checkFilePermission(const QString &path);

    /**
     * @brief showWarningDialog 通用的警告对话框
     * @param msg
     * @param strTitle
     * @return
     */
    int showWarningDialog(const QString &msg, const QString &strTitle = "");

signals:
    void signalCompressClicked(const QVariant &val);

private slots:
    /**
     * @brief slotShowRightMenu 显示类型菜单
     * @param e 鼠标点击事件
     */
    void slotShowRightMenu(QMouseEvent *e);

    /**
     * @brief slotTypeChanged   压缩格式变化
     * @param action    菜单项
     */
    void slotTypeChanged(QAction *action);

    /**
     * @brief slotFileNameChanged   文件名变化切换风格
     * @param strText   内容
     */
    void slotFileNameChanged(const QString &strText);

    /**
     * @brief slotAdvancedEnabled   设置是否启用高级选项
     * @param bEnabled  是否启用
     */
    void slotAdvancedEnabled(bool bEnabled);

    /**
     * @brief slotSplitEdtEnabled   设置分卷输入框是否启用
     * @param iStatue  勾选状态
     */
    void slotSplitEdtEnabled();

    /**
     * @brief slotCompressClicked   压缩按钮点击
     */
    void slotCompressClicked();

    /**
     * @brief slotCommentTextChanged    注释内容变化
     */
    void slotCommentTextChanged();

private:
    // 左侧界面
    DLabel *m_pTypePixmapLbl;        // 类型图标
    TypeLabel *m_pClickLbl;     // 点击区域
    TypeLabel *m_pCompressTypeLbl;   // 压缩类型
    TypeLabel *pArrowPixmapLbl;    // 菜单箭头z

    // 右侧界面
    DLineEdit *m_pFileNameEdt;  // 压缩包名称
    DFileChooserEdit *m_pSavePathEdt;   // 保存路径
    CustomSwitchButton *m_pAdvancedBtn;    // 高级选项
    DLabel *m_pEncryptedLbl;        // 加密文件标签
    DPasswordEdit *m_pPasswordEdt;  // 密码框
    DLabel *m_pListEncryptionLbl;    // 列表加密标签
    CustomSwitchButton *m_pListEncryptionBtn;  // 列表加密切换
    CustomCheckBox *m_pSplitCkb;    // 分卷压缩选择
    DDoubleSpinBox *m_pSplitValueEdt; // 分卷大小输入框

    DLabel *m_pCompressLevelLbl;      // 压缩方式选择项标签
    DComboBox *m_pCompressLevelCmb;       // 压缩方式选择项
    DLabel *m_pCommentLbl;                  // 注释文字标签
    DTextEdit *m_pCommentEdt;       // 注释信息

    // 压缩按钮
    CustomPushButton *m_pCompressBtn;

    QStringList m_listSupportedMimeTypes;   // 支持压缩的格式
    QMenu *m_pTypeMenu;     // 类型菜单

    QStringList m_listFiles;    // 待压缩文件
    qint64 m_qFileSize;     // 待压缩文件大小

    QString m_strMimeType;  // 压缩类型（application/x-tar）
};

#endif // COMPRESSSETTINGPAGE_H
