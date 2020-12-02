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
#include "datamanager.h"

#include <QAtomicPointer>
#include <QDebug>

//静态成员变量初始化。
QMutex DataManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<DataManager> DataManager::m_instance = nullptr;//原子指针，默认初始化是0

//Q_GLOBAL_STATIC(DataManager, datamanager)

void DataManager::resetArchiveData()
{
    m_stArchiveData.reset();
}

ArchiveData &DataManager::archiveData()
{
    return m_stArchiveData;
}

DataManager::DataManager()
{

}

DataManager::DataManager(const DataManager &)
{

}

DataManager &DataManager::get_instance(void)
{

#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<DataManager>::isTestAndSetNative()) //运行时检测
        qDebug() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new DataManager);//第二次检测。
    }

    return *m_instance;

    //return datamanager();
}
