
#ifndef ARCHIVESORTFILTERMODEL_H
#define ARCHIVESORTFILTERMODEL_H


#include <QSortFilterProxyModel>

class ArchiveSortFilterModel: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ArchiveSortFilterModel(QObject *parent = nullptr);
    ~ArchiveSortFilterModel() override;

    bool lessThan(const QModelIndex &leftIndex, const QModelIndex &rightIndex) const override;
};

#endif // ARCHIVESORTFILTERMODEL_H
