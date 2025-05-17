#include "measurement_sort_filter_proxy_model.h"
#include "measurement_tree_model.h"
#include <QDateTime>

MeasurementSortFilterProxyModel::MeasurementSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
    setSortRole(Qt::DisplayRole);
    setDynamicSortFilter(true);
}

bool MeasurementSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    bool leftIsTopLevel = left.parent().isValid();
    bool rightIsTopLevel = right.parent().isValid();

    if((!leftIsTopLevel || !rightIsTopLevel) && (sourceModel()->hasChildren(left) || sourceModel()->hasChildren(right)))
        return false;

    bool leftHasChildren = sourceModel()->hasChildren(left);
    bool rightHasChildren = sourceModel()->hasChildren(right);

    if(leftHasChildren && !rightHasChildren)
        return true;
    if(!leftHasChildren && rightHasChildren)
        return false;


    if (left.column() == MeasurementTreeModel::Date && right.column() == MeasurementTreeModel::Date)
    {
        QDateTime leftDate = QDateTime::fromString(leftData.toString(), "dd-MM-yyyy hh:mm");
        QDateTime rightDate = QDateTime::fromString(rightData.toString(), "dd-MM-yyyy hh:mm");

        if(leftDate.isValid() && rightDate.isValid())
            return leftDate < rightDate;
    }

    if(leftData.typeId() == QMetaType::QString && rightData.typeId() == QMetaType::QString)
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;

    if(leftData.typeId() == QMetaType::Double || leftData.typeId() == QMetaType::Int)
        return leftData.toDouble() < rightData.toDouble();

    return leftData.toString() < rightData.toString();
}

void MeasurementSortFilterProxyModel::setFilterColumn(int column)
{
    QSortFilterProxyModel::setFilterKeyColumn(column);
}

void MeasurementSortFilterProxyModel::setFilter(const QString& text)
{
    filterString = text;
    invalidateFilter();
}
