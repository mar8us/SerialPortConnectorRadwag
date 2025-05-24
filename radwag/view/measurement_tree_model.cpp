#include "measurement_tree_model.h"
#include <QBrush>
#include <QIcon>
#include <QDateTime>


MeasurementTreeModel::MeasurementTreeModel(MeasurementManager* manager, QObject* parent)
    : QAbstractItemModel(parent)
    , measurementManager(manager)
    , rootItem(new TreeItem())
    , groupByOption(-1)
    , filterText("")
    , filterColumn(-1)
    , currentType(MeasurementType::TwoStage)
{
    connect(measurementManager, &MeasurementManager::measurementAdded, this, &MeasurementTreeModel::onMeasurementChanged);
    connect(measurementManager, &MeasurementManager::measurementRemoved, this, &MeasurementTreeModel::onMeasurementChanged);

    buildTree();
}

MeasurementTreeModel::~MeasurementTreeModel()
{
    delete rootItem;
}

QModelIndex MeasurementTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = nullptr;
    if(row >= 0 && row < parentItem->children.size())
        childItem = parentItem->children.at(row);

    if(childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex MeasurementTreeModel::parent(const QModelIndex& index) const
{
    if(!index.isValid())
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parent;

    if(parentItem == rootItem)
        return QModelIndex();

    TreeItem* grandparentItem = parentItem->parent;
    if(!grandparentItem)
        return QModelIndex();

    int row = grandparentItem->children.indexOf(parentItem);

    return createIndex(row, 0, parentItem);
}

int MeasurementTreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem;

    if(parent.column() > 0)
        return 0;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->children.size();
}

int MeasurementTreeModel::columnCount(const QModelIndex&) const
{
    return ColumnCount;
}

QVariant MeasurementTreeModel::data(const QModelIndex& index, int role) const
{

    if(!index.isValid())
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    if(item->measurementIds.isEmpty())
    {
        if (role == Qt::DisplayRole && index.column() == 0)
        {
            return item->groupKey;
        }
        else if (role == Qt::FontRole && index.column() == 0)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        return QVariant();
    }

    QString measurementId = item->measurementIds.first();
    auto measurement = measurementManager->getMeasurement(measurementId);

    if(!measurement)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case MeasureType:
                return measurement->isThreeType() ? "Trzystopniowy" : "Dwustopniowy";

            case SampleId:
                return measurement->getSample().getId();

            case SampleName:
                return measurement->getSample().getName();

            case Material:
                return measurement->getSample().getMaterialName();

            case Fluid:
                return measurement->getFluidName();

            case Status:
            {
                switch (measurement->getStatus())
                {
                    case MeasurementStatus::InProgress:
                        return tr("W trakcie");
                    case MeasurementStatus::Completed:
                        return tr("Zakończony");
                    case MeasurementStatus::Error:
                        return tr("Błąd");
                    default:
                        return tr("Nieznany");
                }
            }

            case Author:
                return measurement->getAuthor();

            case Date:
                return measurement->getDate().toString("dd-MM-yyyy hh:mm");

            default:
                return QVariant();
        }
    }
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
        switch (measurement->getStatus())
        {
            case MeasurementStatus::InProgress:
                return QIcon(":/icons/measure_progress.png");
            case MeasurementStatus::Completed:
                return QIcon(":/icons/measure_completed.png");
            case MeasurementStatus::Error:
                return QIcon(":/icons/measure_error.png");
            default:
                return QVariant();
        }
    }
    else if (role == Qt::ForegroundRole)
    {
        if(measurement->getStatus() == MeasurementStatus::Error)
            return QBrush(Qt::red);
        else if (measurement->getStatus() == MeasurementStatus::Completed)
            return QBrush(Qt::green);
        return QVariant();
    }
    else if (role == Qt::ToolTipRole)
    {
        return tr("ID: %1\nTyp: %2\nData: %3\nAutor: %4\nStatus: %5")
            .arg(measurement->getId())
            .arg(measurement->isThreeType() ? tr("Trójstopniowy") : tr("Dwustopniowy"))
            .arg(measurement->getDate().toString("dd-MM-yyyy hh:mm"))
            .arg(measurement->getAuthor())
            .arg(measurement->getStatus() == MeasurementStatus::Completed ? tr("Zakończony") : (measurement->getStatus() == MeasurementStatus::InProgress ? tr("W trakcie") : tr("Błąd")));
    }

    return QVariant();
}

QVariant MeasurementTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case MeasureType:
                return tr("Typ pomiaru");

            case SampleId:
                return tr("ID próbki");

            case SampleName:
                return tr("Nazwa próbki");

            case Material:
                return tr("Materiał");

            case Fluid:
                return tr("Ciecz");

            case Status:
                return tr("Status");

            case Author:
                return tr("Wykonawca");

            case Date:
                return tr("Data");

            default:
                return QVariant();
        }
    }

    return QVariant();
}

void MeasurementTreeModel::setGroupBy(int groupByIndex)
{
    if(groupByOption == groupByIndex)
        return;

    groupByOption = groupByIndex;
    buildTree();
}

void MeasurementTreeModel::setFilterText(const QString& text)
{
    if(filterText == text)
        return;

    filterText = text;
    buildTree();
}

void MeasurementTreeModel::setFilterColumn(int column)
{
    if(filterColumn == column)
        return;

    filterColumn = column;
    buildTree();
}

void MeasurementTreeModel::setMeasurementType(MeasurementType type)
{
    if(currentType == type)
        return;
    currentType = type;
    buildTree();

}

void MeasurementTreeModel::refresh()
{
    buildTree();
}

void MeasurementTreeModel::onMeasurementChanged(const QString&)
{
    buildTree();
}

void MeasurementTreeModel::clearTree()
{
    beginResetModel();
    delete rootItem;
    rootItem = new TreeItem();
    endResetModel();
}

void MeasurementTreeModel::buildTree()
{
    beginResetModel();

    delete rootItem;
    rootItem = new TreeItem();

    QMap<QString, TreeItem*> groupItems;

    const auto& measurements = measurementManager->getMeasurements();
    for(auto it = measurements.begin(); it != measurements.end(); it++)
    {
        auto measurement = it.value();

        if(!matchesFilter(measurement, filterColumn, filterText))
            continue;

        if(groupByOption == -1)
        {
            TreeItem* leafItem = new TreeItem(QString(), rootItem);
            leafItem->measurementIds.append(measurement->getId());
            rootItem->children.append(leafItem);
        }
        else
        {
            QString groupKey = getGroupKey(measurement, groupByOption);
            if(!groupItems.contains(groupKey))
            {
                TreeItem* groupItem = new TreeItem(groupKey, rootItem);
                rootItem->children.append(groupItem);
                groupItems[groupKey] = groupItem;
            }

            TreeItem* leafItem = new TreeItem(QString(), groupItems[groupKey]);
            leafItem->measurementIds.append(measurement->getId());
            groupItems[groupKey]->children.append(leafItem);
        }
    }

    endResetModel();
}

MeasurementTreeModel::TreeItem* MeasurementTreeModel::getItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

QString MeasurementTreeModel::getStatusText(MeasurementStatus status) const
{
    switch(status)
    {
        case MeasurementStatus::InProgress:
            return tr("W trakcie");
        case MeasurementStatus::Completed:
            return tr("Zakończony");
        case MeasurementStatus::Error:
            return tr("Błąd");
        default:
            return tr("Nieznany");
    }
}

bool MeasurementTreeModel::matchesFilter(const std::shared_ptr<const Measurement>& measurement, int filterColumn, const QString& filterText) const
{
    if (filterText.isEmpty())
        return true;

    switch(filterColumn)
    {
        case MeasureType:
            return QString(measurement->isThreeType() ? "Trzystopniowy" : "Dwustopniowy")
                .contains(filterText, Qt::CaseInsensitive);

        case SampleId:
            return measurement->getSample().getId().contains(filterText, Qt::CaseInsensitive);

        case SampleName:
            return measurement->getSample().getName().contains(filterText, Qt::CaseInsensitive);

        case Material:
            return measurement->getSample().getMaterialName().contains(filterText, Qt::CaseInsensitive);

        case Fluid:
            return measurement->getFluidName().contains(filterText, Qt::CaseInsensitive);

        case Status:
            return getStatusText(measurement->getStatus()).contains(filterText, Qt::CaseInsensitive);

        case Author:
            return measurement->getAuthor().contains(filterText, Qt::CaseInsensitive);

        case Date:
            return measurement->getDate().toString("dd-MM-yyyy hh:mm").contains(filterText, Qt::CaseInsensitive);

        default:
            return QString(measurement->isThreeType() ? "Pomiar trzystopniowy" : "Pomiar dwustopniowy").contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getId().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getSample().getId().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getSample().getName().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getSample().getMaterialName().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getFluidName().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getAuthor().contains(filterText, Qt::CaseInsensitive) ||
                   measurement->getDate().toString("dd-MM-yyyy hh:mm").contains(filterText, Qt::CaseInsensitive) ||
                   getStatusText(measurement->getStatus()).contains(filterText, Qt::CaseInsensitive);
        }
}

QString MeasurementTreeModel::getGroupKey(const std::shared_ptr<const Measurement>& measurement, int groupByOption) const
{
    switch (groupByOption)
    {
        case MeasureType:
            return measurement->isThreeType() ? tr("Trzystopniowy") : tr("Dwustopniowy");

        case SampleId:
            return measurement->getSample().getId();

        case Material:
            return measurement->getSample().getMaterialName();

        case Author:
            return measurement->getAuthor();

        case SampleName:
            return measurement->getSample().getName();

        case Fluid:
            return measurement->getFluidName();

        case Status:
            return getStatusText(measurement->getStatus());

        case Date:
            return measurement->getDate().toString("dd-MM-yyyy");

        default:
            return QString();
    }
}
