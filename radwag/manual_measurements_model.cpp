#include "manual_measurements_model.h"
#include <QBrush>
#include <QFont>

ManualMeasurementsModel::ManualMeasurementsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new TreeItem())
    , m_groupByColumn(-1)
{
    buildTree();
}

ManualMeasurementsModel::~ManualMeasurementsModel()
{
    delete m_rootItem;
}

QModelIndex ManualMeasurementsModel::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    if(!parent.isValid())
        parentItem = m_rootItem;
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

QModelIndex ManualMeasurementsModel::parent(const QModelIndex& index) const
{
    if(!index.isValid())
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parent;

    if(parentItem == m_rootItem)
        return QModelIndex();

    TreeItem* grandparentItem = parentItem->parent;
    if(!grandparentItem)
        return QModelIndex();

    int row = grandparentItem->children.indexOf(parentItem);

    return createIndex(row, 0, parentItem);
}

int ManualMeasurementsModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem;

    if(parent.column() > 0)
        return 0;

    if(!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->children.size();
}

int ManualMeasurementsModel::columnCount(const QModelIndex&) const
{
    return ColumnCount;
}

QVariant ManualMeasurementsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    // Jeśli to węzeł grupujący (nie ma pomiarów, tylko groupKey)
    if(item->measurements.isEmpty())
    {
        if(role == Qt::DisplayRole && index.column() == 0)
        {
            return item->groupKey.isEmpty() ? tr("Bez etykiety") : item->groupKey;
        }
        else if(role == Qt::FontRole && index.column() == 0)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        return QVariant();
    }

    // Węzeł z pomiarem
    const ManualMeasurementRecord& record = item->measurements.first();

    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
            case DateTime:
                return record.timestamp.toString("yyyy-MM-dd HH:mm:ss");

            case Value:
                return QString::number(record.value, 'f', 4);

            case Unit:
                return record.unit;

            case Stability:
                return record.isStable ? tr("TAK") : tr("NIE");

            case Label:
                return record.label.isEmpty() ? tr("---") : record.label;

            default:
                return QVariant();
        }
    }
    else if(role == Qt::ForegroundRole && index.column() == Stability)
    {
        return record.isStable ? QBrush(Qt::darkGreen) : QBrush(Qt::red);
    }
    else if(role == Qt::ToolTipRole)
    {
        return tr("Data: %1\nWartość: %2 %3\nStabilny: %4\nEtykieta: %5\nNotatka: %6")
            .arg(record.timestamp.toString("dd-MM-yyyy HH:mm:ss"))
            .arg(record.value, 0, 'f', 4)
            .arg(record.unit)
            .arg(record.isStable ? tr("TAK") : tr("NIE"))
            .arg(record.label.isEmpty() ? tr("brak") : record.label);
    }

    return QVariant();
}

QVariant ManualMeasurementsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
            case DateTime:
                return tr("Data/Czas");

            case Value:
                return tr("Wartość");

            case Unit:
                return tr("Jednostka");

            case Stability:
                return tr("Stabilność");

            case Label:
                return tr("Etykieta");

            default:
                return QVariant();
        }
    }

    return QVariant();
}

void ManualMeasurementsModel::addMeasurement(const ManualMeasurementRecord& record)
{
    m_measurements.append(record);
    buildTree();
}

void ManualMeasurementsModel::removeMeasurements(const QModelIndexList& indexes)
{
    // TODO: implementacja usuwania
}

void ManualMeasurementsModel::clearAll()
{
    m_measurements.clear();
    buildTree();
}

void ManualMeasurementsModel::assignLabelToIndexes(const QModelIndexList& indexes, const QString& label)
{
    // TODO: implementacja przypisywania etykiet
}

QList<ManualMeasurementRecord> ManualMeasurementsModel::getAllMeasurements() const
{
    return m_measurements.toList();
}

void ManualMeasurementsModel::setGroupBy(int columnEnum)
{
    if(m_groupByColumn == columnEnum)
        return;

    m_groupByColumn = columnEnum;
    buildTree();
}

void ManualMeasurementsModel::refresh()
{
    buildTree();
}

QString ManualMeasurementsModel::getGroupKey(const ManualMeasurementRecord& record, int columnEnum) const
{
    switch(columnEnum)
    {
        case DateTime:
            return record.timestamp.toString("yyyy-MM-dd");

        case Value:
            return QString::number(record.value, 'f', 4);

        case Unit:
            return record.unit;

        case Stability:
            return record.isStable ? tr("Stabilne") : tr("Niestabilne");

        case Label:
            return record.label.isEmpty() ? tr("Bez etykiety") : record.label;

        default:
            return QString();
    }
}

void ManualMeasurementsModel::buildTree()
{
    beginResetModel();

    delete m_rootItem;
    m_rootItem = new TreeItem();

    if(m_groupByColumn == -1)
    {
        for(const auto& measurement : m_measurements)
        {
            TreeItem* leafItem = new TreeItem(QString(), m_rootItem);
            leafItem->measurements.append(measurement);
            m_rootItem->children.append(leafItem);
        }
    }
    else
    {
        QMap<QString, TreeItem*> groupItems;

        for(const auto& measurement : m_measurements)
        {
            QString groupKey = getGroupKey(measurement, m_groupByColumn);

            if(!groupItems.contains(groupKey))
            {
                TreeItem* groupItem = new TreeItem(groupKey, m_rootItem);
                m_rootItem->children.append(groupItem);
                groupItems[groupKey] = groupItem;
            }

            TreeItem* leafItem = new TreeItem(QString(), groupItems[groupKey]);
            leafItem->measurements.append(measurement);
            groupItems[groupKey]->children.append(leafItem);
        }
    }

    endResetModel();
}

void ManualMeasurementsModel::clearTree()
{
    beginResetModel();
    delete m_rootItem;
    m_rootItem = new TreeItem();
    endResetModel();
}

ManualMeasurementsModel::TreeItem* ManualMeasurementsModel::getItem(const QModelIndex& index) const
{
    if(!index.isValid())
        return nullptr;

    return static_cast<TreeItem*>(index.internalPointer());
}
