#ifndef MYFILEITEM_H
#define MYFILEITEM_H

#include <QStandardItemModel>
#include "utils.h"


class MyFileItem : public QStandardItem
{

public:
    MyFileItem();
    MyFileItem(const QString &text);
    MyFileItem(const QIcon &icon, const QString &text);


    bool operator<(const QStandardItem &other) const override;
};

#endif // MYFILEITEM_H
