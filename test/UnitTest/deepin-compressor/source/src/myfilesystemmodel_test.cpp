/*
* Copyright (C) 2019 ~ %YEAR% Uniontech Software Technology Co.,Ltd.
*
* Author:     tenghuan <tenghuan@uniontech.com>
*
* Maintainer: tenghuan <tenghuan@uniontech.com>
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

#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include <iostream>
#include <QList>
#include "myfilesystemmodel.h"

TEST(MyFileSystemModel_MyFileSystemModel, MyFileSystemModel_MyFileSystemModel_UT001)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    EXPECT_NE(mfModel, nullptr);
    delete  mfModel;
}

TEST(MyFileSystemModel_setPathIndex, MyFileSystemModel_setPathIndex_UT001)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    int index = 10;
    mfModel->setPathIndex(&index);
    EXPECT_EQ(*(mfModel->ppathindex), 10);
    delete mfModel;
}

TEST(MyFileSystemModel_setTableView, MyFileSystemModel_setTableView_UT001)
{
    DTableView *tbview = new DTableView;
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    mfModel->setTableView(tbview);
    EXPECT_NE(mfModel->m_tableview, nullptr);
    delete tbview;
    delete mfModel;
}

bool myremove(const QModelIndex &index)
{
    return true;
}

//TEST(MyFileSystemModel_deleteRows,MyFileSystemModel_deleteRows_UT001)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    QModelIndexList indexList;
//    indexList << index;
//    Stub *stub = new Stub;
//    stub->set(ADDR(MyFileSystemModel,remove),myremove);
//    mfModel->deleteRows(indexList);
//    delete mfModel;
//    delete  stub;
//}


TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT001)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(0, Qt::Orientation::Vertical, Qt::DisplayRole);
    const char *str = data.toString().toLocal8Bit().data();
    EXPECT_STREQ(str, "Name");
    delete mfModel;

}

TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT002)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(3, Qt::Orientation::Vertical, Qt::DisplayRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "Size");
    delete mfModel;

}


TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT003)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(2, Qt::Orientation::Vertical, Qt::DisplayRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "Type");
    delete mfModel;

}

TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT004)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(1, Qt::Orientation::Vertical, Qt::DisplayRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "Time modified");
    delete mfModel;

}

TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT005)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(5, Qt::Orientation::Vertical, Qt::DisplayRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "-");
    delete mfModel;

}


TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT006)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(5, Qt::Orientation::Vertical, Qt::TextAlignmentRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "129");
    delete mfModel;

}

TEST(MyFileSystemModel_headerData, MyFileSystemModel_headerData_UT007)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QVariant data = mfModel->headerData(5, Qt::Orientation::Vertical, Qt::DecorationRole);
    QString str = data.toString();
    EXPECT_STREQ(str.toLocal8Bit().data(), "");
    delete mfModel;
}
bool my_isValid1()
{
    return false;
}
TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT001)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    QModelIndex index;
    index.r = 1;
    index.c = 0;
    Stub *stub = new Stub;
    stub->set(ADDR(QModelIndex, isValid), my_isValid1);
    auto val = mfModel->data(index, 0);
    QString str = val.toString();
    EXPECT_STREQ(str.toStdString().c_str(), "");
    delete  stub;
    delete mfModel;

}
bool my_isValid()
{
    return true;
}
QFileInfo myFileInfo(const QModelIndex &index)
{
    return QFileInfo("/home/lx777/mydemo/test.zip");
}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT002)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::TextColorRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT003)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::TextAlignmentRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "129");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT004)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::TextAlignmentRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "129");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT005)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 0;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::FontRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "Noto Sans CJK SC,-1,12,5,57,0,0,0,0,0");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT006)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 1;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::FontRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "Noto Sans CJK SC,-1,11,5,50,0,0,0,0,0");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT007)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 3;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "314.0 B");
//    delete stub;
//    delete mfModel;
//}
//QFileInfo myFileInfo1(const QModelIndex &index)
//{
//    return QFileInfo("/home/lx777/mydemo");
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT008)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 3;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "4 项");
//    delete stub;
//    delete mfModel;
//}

//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT009)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 2;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "目录");
//    delete stub;
//    delete mfModel;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT010)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 1;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT011)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 0;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "mydemo");
//    delete stub;
//    delete mfModel;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT012)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 5;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DisplayRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT013)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 0;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DecorationRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT014)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 1;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    auto val = mfModel->data(index, Qt::DecorationRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}
//bool myIsNull()
//{
//    return true;
//}
//TEST(MyFileSystemModel_data, MyFileSystemModel_data_UT015)
//{
//    MyFileSystemModel *mfModel = new MyFileSystemModel;
//    QModelIndex index;
//    index.c = 0;
//    Stub *stub = new Stub;
//    stub->set(ADDR(QModelIndex, isValid), my_isValid);
//    stub->set(ADDR(MyFileSystemModel, fileInfo), myFileInfo1);
//    stub->set(ADDR(QIcon, isNull), myIsNull);
//    auto val = mfModel->data(index, Qt::DecorationRole);
//    QString str = val.toString();
//    EXPECT_STREQ(str.toStdString().c_str(), "");
//    delete stub;
//    delete mfModel;
//}


TEST(MyFileSystemModel_sort, MyFileSystemModel_sort_UT001)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    mfModel->sort(1, Qt::SortOrder::AscendingOrder);
}

TEST(MyFileSystemModel_sort, MyFileSystemModel_sort_UT002)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    mfModel->sort(3, Qt::SortOrder::AscendingOrder);
}
TEST(MyFileSystemModel_sort, MyFileSystemModel_sort_UT003)
{
    MyFileSystemModel *mfModel = new MyFileSystemModel;
    mfModel->sort(0, Qt::SortOrder::AscendingOrder);
}




