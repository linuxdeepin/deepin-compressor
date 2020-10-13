#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#endif // SETTINGDIALOG_H

#include <DSettingsDialog>
#include <DSettings>
#include <DSettingsOption>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class SettingDialog: public DSettingsDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog() override;

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
     * @brief createSettingButton   创建选择按钮
     */
    void createSettingButton();

    /**
     * @brief createPathBox    创建默认解压位置选项
     */
    void createPathBox();

    /**
     * @brief createDeleteBox   创建删除选项
     */
    void createDeleteBox();

signals:
    /**
     * @brief sigResetPath    点击恢复默认按钮，恢复默认解压路径设置的信号
     */
    void sigResetPath();

    /**
     * @brief sigResetDeleteArchive   点击恢复默认按钮，恢复默认解压后删除压缩文件设置的信号
     */
    void sigResetDeleteArchive();

private slots:
    /**
     * @brief settingsChanged   DSetting value发生改变
     * @param key   发生改变的key
     * @param value   key所对应改变后的值
     */
    void slotSettingsChanged(const QString &key, const QVariant &value);

    /**
     * @brief slotClickSelectAllButton   点击全选按钮
     */
    void slotClickSelectAllButton();

    /**
     * @brief slotClickCancelSelectAllButton    点击取消全选按钮
     */
    void slotClickCancelSelectAllButton();

    /**
     * @brief slotClickRecommendedButton    点击推荐选择按钮
     */
    void slotClickRecommendedButton();

private:
    DSettings *m_settings;          //
    QStringList m_associtionList;   //

    DSettingsOption *m_customButtonOption;   // 按钮选项
    DSettingsOption *m_extractPathOption;     // 默认解压路径选项
    DSettingsOption *m_deleteArchiveOption;    // 解压后删除压缩文件选项

    QString m_curpath;
    int m_index_last;
    QString m_autoDeleteArchive;    // 解压后删除压缩文件方式

};
