// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
#include <QCoreApplication>
#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <type_traits>
#endif

namespace _Singleton {
template<typename T>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static typename std::enable_if<QtPrivate::AreArgumentsCompatible<T, QObject>::value, void>::type
#else
static typename QtPrivate::QEnableIf<QtPrivate::AreArgumentsCompatible<T, QObject>::value>::Type
#endif
handleQObject(QObject *object)
{
    if (qApp) {
        object->moveToThread(qApp->thread());
    }
}

template<typename T>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static typename std::enable_if<!QtPrivate::AreArgumentsCompatible<T, QObject>::value, void>::type
#else
static typename QtPrivate::QEnableIf<!QtPrivate::AreArgumentsCompatible<T, QObject>::value>::Type
#endif
handleQObject(void*) {}
}

template<typename T>
class Singleton
{
public:
    static T *instance() {
        static T instance;

        if (QtPrivate::AreArgumentsCompatible<T, QObject>::value) {
            _Singleton::handleQObject<T>(&instance);
        }

        return &instance;
    }

private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton &);
    Singleton & operator = (const Singleton &);
};

#endif // SINGLETON_H
