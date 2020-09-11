//
// Created by lx777 on 2020/8/28.
//
//#define private public
#include <gtest/gtest.h>
#include "archivemodel.h"
#include <gtest/src/stub.h>
#include <iostream>
#include <QList>

int returnAt = 0;


TEST(ArchiveModel_ArchiveModel_UT,ArchiveModel_ArchiveModel_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ASSERT_NE(model,nullptr);
    delete  model;
}

int myrow()
{
    return 1;
}
void *myInternalPointer()
{
    Archive::Entry *entry = new Archive::Entry(nullptr,"/home/th/my.rar");
    return entry;
}

int myat(int n)
{
    return returnAt;
}
int mycolumn()
{
    return 0;
}
bool myisValid()
{
    return true;
}


TEST(ArchiveModel_Data_UT,ArchiveModel_Data_Ut001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    index.r = 0;
    index.c = 0;
    for(int i= 0; i < 10; ++i)
    {
        model->m_showColumns.append(i);
    }
    Stub *stub = new Stub;
    stub->set(ADDR(QModelIndex,internalPointer),myInternalPointer);
    stub->set(ADDR(QModelIndex,isValid),myisValid);

    returnAt = 0;
    QString name = model->data(index,Qt::DisplayRole).toString();
    std::cout << name.toStdString() << std::endl;
    ASSERT_EQ(name.toStdString(),"my.rar");
    stub->reset(ADDR(QModelIndex,internalPointer));
    stub->reset(ADDR(QModelIndex,isValid));
    delete  stub;
    delete model;

}

TEST(ArchiveModel_Data_UT,ArchiveModel_Data_Ut002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    index.r = 1;
    index.c = 2;
    for(int i= 0; i < 10; ++i)
    {
        model->m_showColumns.append(i);
    }
    Stub *stub = new Stub;
    stub->set(ADDR(QModelIndex,internalPointer),myInternalPointer);
    stub->set(ADDR(QModelIndex,isValid),myisValid);

    returnAt = 0;
    QString name = model->data(index,Qt::DisplayRole).toString();
    std::cout << name.toStdString() << std::endl;
    ASSERT_EQ(name.toStdString(),"压缩文件");
    stub->reset(ADDR(QModelIndex,internalPointer));
    stub->reset(ADDR(QModelIndex,isValid));
    delete  stub;
    delete model;

}

TEST(ArchiveModel_Data_UT,ArchiveModel_Data_Ut003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    index.r = 1;
    index.c = 3;
    for(int i= 0; i < 10; ++i)
    {
        model->m_showColumns.append(i);
    }
    Stub *stub = new Stub;
    stub->set(ADDR(QModelIndex,internalPointer),myInternalPointer);
    stub->set(ADDR(QModelIndex,isValid),myisValid);

    returnAt = 0;
    QString name = model->data(index,Qt::DisplayRole).toString();
    std::cout << name.toStdString() << std::endl;
    ASSERT_EQ(name.toStdString(),"压缩文件");
    stub->reset(ADDR(QModelIndex,internalPointer));
    stub->reset(ADDR(QModelIndex,isValid));
    delete  stub;
    delete model;

}



//TEST(ArchiveModel_cleanFileName_UT,ArchiveModel_cleanFileName_UT001)
//{
//    ArchiveModel *mode =  new ArchiveModel;
//    mode->cleanFileName("");
//}


