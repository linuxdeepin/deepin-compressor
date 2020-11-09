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
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "commonstruct.h"

#include <QObject>
#include <QMutex>

class DataManager
{
public:
    //DataManager();//防止构造函数
    /**
     * @brief get_instance  获取单例实例对象
     * @return
     */
    static DataManager &get_instance(void);

    /**
     * @brief resetArchiveData  重置数据
     */
    void resetArchiveData();

    /**
     * @brief archiveData   获取压缩包数据
     * @return  数据
     */
    ArchiveData &archiveData();

private:
    DataManager();//防止构造函数
    DataManager(const DataManager &);//防止拷贝构造函数
    DataManager &operator=(const DataManager &); //防止赋值拷贝构造函数

    ArchiveData m_stArchiveData;        // 存储当前操作的压缩包数据

    static QMutex m_mutex;//实例互斥锁。
    static QAtomicPointer<DataManager> m_instance;/*!<使用原子指针,默认初始化为0。*/
};



#endif // DATAMANAGER_H
