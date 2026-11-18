// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_KPLUGINFACTORY_H
#define OWN_KPLUGINFACTORY_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

#include <functional>

/**
 * @brief 插件工厂基类。
 *
 * 插件壳继承本类，在构造函数里 registerPlugin<Xxx>() 声明自己能造出哪个后端；
 * 宿主通过 KPluginLoader::factory()->create<ReadOnlyArchiveInterface>(...) 取实例。
 * 语义与 3rdparty/interface/plugin 里的 KPluginFactory 一致，只保留本工程用到的部分。
 */
class KPluginFactory : public QObject
{
    Q_OBJECT

public:
    ~KPluginFactory() override;

    template<typename T>
    T *create(QObject *parent = nullptr, const QVariantList &args = QVariantList())
    {
        QObject *object = createInstance(T::staticMetaObject.className(), parent, args, QString());
        T *result = static_cast<T *>(object);
        if (!result)
            delete object;
        return result;
    }

    template<typename T>
    T *create(const QString &keyword, QObject *parent = nullptr, const QVariantList &args = QVariantList())
    {
        QObject *object = createInstance(T::staticMetaObject.className(), parent, args, keyword);
        T *result = static_cast<T *>(object);
        if (!result)
            delete object;
        return result;
    }

protected:
    explicit KPluginFactory(QObject *parent = nullptr);

    template<typename T>
    void registerPlugin(const QString &keyword = QString())
    {
        Creator creator = [](QObject *parent, const QVariantList &args) -> QObject * {
            return new T(parent, args);
        };
        m_creators.append(Entry{&T::staticMetaObject, creator, keyword});
    }

    QObject *createInstance(const char *className, QObject *parent,
                            const QVariantList &args, const QString &keyword);

private:
    using Creator = std::function<QObject *(QObject *, const QVariantList &)>;
    struct Entry
    {
        const QMetaObject *meta;
        Creator creator;
        QString keyword;
    };

    QVector<Entry> m_creators;
};

// 与 Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory") 配对，插件 .so 才能被 qobject_cast 成工厂
Q_DECLARE_INTERFACE(KPluginFactory, "org.kde.KPluginFactory")

#endif // OWN_KPLUGINFACTORY_H
