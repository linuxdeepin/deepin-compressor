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

TEST(ArchiveModel_ArchiveModel_UT, ArchiveModel_ArchiveModel_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ASSERT_NE(model, nullptr);
    delete model;
}

int myrow()
{
    return 1;
}
void *myInternalPointer()
{
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/th/my.rar");
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
/*
TEST(ArchiveModel_Data_UT,ArchiveModel_Data_Ut001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    Stub *stub = new Stub;
    stub->set(ADDR(QModelIndex,row),myrow);
    stub->set(ADDR(QModelIndex,internalPointer),myInternalPointer);
    stub->set(ADDR(QModelIndex,isValid),myisValid);
    stub->set(ADDR(QModelIndex,column),mycolumn);
    stub->set(ADDR(QList<int>,at),myat);
    returnAt = 0;
    QString name = model->data(index,0).toString();
    std::cout << name.toStdString() << std::endl;
    ASSERT_EQ(name.toStdString(),"/home/th/my.rar");
    delete  stub;
    delete model;

}*/
