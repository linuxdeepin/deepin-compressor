/*
 * Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CLLLAYERPLUGIN_H
#define CLLLAYERPLUGIN_H

#include "archiveinterface.h"
#include "kpluginfactory.h"

class QFile;
class QTemporaryDir;

class LayerPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clilllayer.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit LayerPluginFactory();
    ~LayerPluginFactory() override;
};

/**
 * @brief 玲珑（linglong）layer/uab 包只读插件
 *
 * 复刻官方 linglong 库（libs/linglong）的解包逻辑：
 * - layer: [40B 魔数][4B LE 长度][LayerInfo JSON][EROFS 数据至文件尾]
 *          对应官方 LayerFile::binaryDataOffset() = 40 + metaLen + 4
 * - uab:   ELF64 容器，section "linglong.meta"（JSON，内含 sections.bundle
 *          指向存放 EROFS 镜像的 section 名）对应官方 UABFile::unpack()
 * 解压使用官方同款兜底工具 `fsck.erofs --extract=<dir> <erofs>`（无 root、
 * 无挂载要求；官方优先 erofsfuse 挂载，无该命令时同样回退 fsck.erofs）。
 */
class LayerPlugin : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit LayerPlugin(QObject *parent, const QVariantList &args);
    ~LayerPlugin() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    void pauseOperation() override {}
    void continueOperation() override {}

private:
    /**
     * @brief extractPackage 将当前 layer/uab 包解压到 destDir
     *        （解析格式头 → 复制 EROFS 载荷到临时文件 → fsck.erofs 解压）
     * @param destDir 目标目录（可为已存在的空目录，fsck.erofs 解压到该目录）
     * @return 空串表示成功，否则为错误信息
     */
    QString extractPackage(const QString &destDir);

    /**
     * @brief parseHeader 解析包头，定位 EROFS 载荷
     * @param erofsOffset 载荷在包文件中的起始偏移
     * @param erofsSize 载荷字节数
     * @param errOut 失败时的错误信息
     * @return true: 成功；false: 格式不识别或文件损坏
     */
    bool parseHeader(qint64 *erofsOffset, qint64 *erofsSize, QString *errOut);

    /**
     * @brief parseLayerHeader 按 layer 布局解析（对齐官方 LayerFile）
     */
    bool parseLayerHeader(QFile &file, qint64 *erofsOffset, qint64 *erofsSize, QString *errOut);

    /**
     * @brief parseUabHeader 按 uab 布局解析 ELF64 section（对齐官方 UABFile）
     */
    bool parseUabHeader(QFile &file, qint64 *erofsOffset, qint64 *erofsSize, QString *errOut);

    /**
     * @brief copyPayload 从包文件 offset 复制 size 字节到 dstPath
     *        （fsck.erofs 老版本不支持 offset 参数，与官方兜底策略一致先落盘）
     */
    bool copyPayload(QFile &file, qint64 offset, qint64 size, const QString &dstPath, QString *errOut);

    /**
     * @brief runFsckExtract 调用 fsck.erofs 将 EROFS 镜像解压到 destDir
     * @param erofsFile EROFS 镜像文件
     * @param destDir 解压目标目录
     * @return 空串表示成功，否则为错误信息
     */
    QString runFsckExtract(const QString &erofsFile, const QString &destDir);

    /**
     * @brief collectEntries 递归收集 dir 下所有文件/目录（含 dir 本身的直接子项），
     *        填充 DataManager 中的归档数据，用于预览列表
     * @param dir 已解压出的包根目录
     */
    void collectEntries(const QString &dir);

    /**
     * @brief moveToDest 将 src 移动（或拷贝）到 dst，处理同名冲突（弹覆盖询问）
     * @return true: 成功（含跳过）；false: 用户取消
     */
    bool moveToDest(const QString &src, const QString &dst);

    /**
     * @brief moveStagedToTarget 把暂存目录中的内容搬到最终解压目录
     * @param stageDir 解压出的包根目录
     * @param targetPath 最终解压目标目录
     * @param extractAll 是否全部解压
     * @param files 选中的文件列表（extractAll 为 false 时使用）
     * @param strDestination 选中项在包内的上级目录（空表示选中项位于包根目录）
     * @return true: 成功；false: 用户取消或失败
     */
    bool moveStagedToTarget(const QString &stageDir, const QString &targetPath, bool extractAll,
                            const QList<FileEntry> &files, const QString &strDestination);

    qlonglong m_qTotalSize = 0;   // 本次解压/提取的总字节数，用于进度计算
    qlonglong m_qDoneSize = 0;    // 已完成的字节数
};

#endif   // CLLLAYERPLUGIN_H
