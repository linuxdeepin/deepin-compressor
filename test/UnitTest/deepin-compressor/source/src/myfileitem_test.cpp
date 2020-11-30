#include <gtest/gtest.h>
#include "myfileitem.h"
#include <gtest/src/stub.h>
#include <iostream>
#include <QList>
#include <QIcon>


TEST(MyFileItem_MyFileItem, MyFileItem_MyFileItem_UT001)
{
    MyFileItem *item = new MyFileItem;
    EXPECT_NE(item, nullptr);
    delete  item;
}

TEST(MyFileItem_MyFileItem, MyFileItem_MyFileItem_UT002)
{
    MyFileItem *item = new MyFileItem("test");
    EXPECT_NE(item, nullptr);
    delete  item;
}
TEST(MyFileItem_MyFileItem, MyFileItem_MyFileItem_UT003)
{
    MyFileItem *item = new MyFileItem(QIcon(), "test");
    EXPECT_NE(item, nullptr);
    delete  item;
}
int nCol = 0;
int mycolumn11()
{
    return nCol;
}
//TEST(MyFileItem_operator,MyFileItem_operator_UT001)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "项";

//    nCol = 3;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "123";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}
//TEST(MyFileItem_operator,MyFileItem_operator_UT002)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "123";

//    nCol = 3;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "项";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_FALSE(is);
//    delete  sub;
//}

//TEST(MyFileItem_operator,MyFileItem_operator_UT003)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "项1";

//    nCol = 3;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "项2";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}
//TEST(MyFileItem_operator,MyFileItem_operator_UT004)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "123";

//    nCol = 3;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "456";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_FALSE(is);
//    delete  sub;
//}

//TEST(MyFileItem_operator,MyFileItem_operator_UT005)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "2020/09/11 09:00:00";

//    nCol = 1;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "2020/09/11 09:01:00";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}
//TEST(MyFileItem_operator,MyFileItem_operator_UT006)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "mmo";

//    nCol = 0;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "rpg";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}


//TEST(MyFileItem_operator,MyFileItem_operator_UT007)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "mmo";

//    nCol = 5;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn11);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "rpg";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}
//int mycolumn22()
//{
//    return nCol++;
//}

//TEST(MyFileItem_operator,MyFileItem_operator_UT008)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "123";

//    nCol = 3;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn22);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "456";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}
//TEST(MyFileItem_operator,MyFileItem_operator_UT009)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "2020/09/11 09:00:00";

//    nCol = 1;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn22);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "2020/09/11 09:01:00";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}


//TEST(MyFileItem_operator,MyFileItem_operator_UT010)
//{
//    MyFileItem item ;
//    QStandardItem item1 ;
//    QVariant ss = "mmo";

//    nCol = 0;
//    Stub *sub = new Stub;
//    sub->set(ADDR(MyFileItem,column),mycolumn22);
//    item.setData(ss,Qt::DisplayRole);
//     ss = "rpg";
//    item1.setData(ss,Qt::DisplayRole);
//  //  item.operator<(item1);
//    bool is =  item < item1;
//    EXPECT_TRUE(is);
//    delete  sub;
//}


