#ifndef MyFILESYSTEMMODEL_H
#define MyFILESYSTEMMODEL_H

#include <QtWidgets/qtwidgetsglobal.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpair.h>
#include <QtCore/qdir.h>
#include <QtGui/qicon.h>
#include <QtCore/qdiriterator.h>
#include <QFileSystemModel>
#include <QTableView>
#include "mimetypedisplaymanager.h"

namespace  MyFileSystemDefine{
    const int  gTableHeight=36;
}

QT_REQUIRE_CONFIG(filesystemmodel);

QT_BEGIN_NAMESPACE

class ExtendedInformation;
class MyFileSystemModelPrivate;
class MyFileIconProvider;

class MyFileSystemModel : public QFileSystemModel
{
    public:
        explicit MyFileSystemModel(QObject *parent = nullptr);
        ~MyFileSystemModel();
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
        void setPathIndex(int *index);
        void setTableView(QTableView *tableview);
    private:
        bool m_showreprevious;
        int *ppathindex;
        QTableView *m_tableview;
        MimeTypeDisplayManager* m_mimetype;

};

#endif // MyFILESYSTEMMODEL_H
