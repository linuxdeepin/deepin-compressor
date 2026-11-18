// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kpluginfactory.h"

#include <QByteArray>

KPluginFactory::KPluginFactory(QObject *parent)
    : QObject(parent)
{
}

KPluginFactory::~KPluginFactory() = default;

QObject *KPluginFactory::createInstance(const char *className, QObject *parent,
                                        const QVariantList &args, const QString &keyword)
{
    const QByteArray wanted(className);

    for (const Entry &entry : m_creators) {
        if (!keyword.isEmpty() && entry.keyword != keyword)
            continue;

        // 注册的是具体实现类，请求的是它的基类（ReadOnly/ReadWriteArchiveInterface），
        // 所以沿 metaobject 继承链往上找
        bool matched = false;
        for (const QMetaObject *meta = entry.meta; meta; meta = meta->superClass()) {
            if (wanted == meta->className()) {
                matched = true;
                break;
            }
        }
        if (!matched)
            continue;

        if (QObject *object = entry.creator(parent, args))
            return object;
    }

    return nullptr;
}
