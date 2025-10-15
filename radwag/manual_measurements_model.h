#ifndef MANUAL_MEASUREMENTS_MODEL_H
#define MANUAL_MEASUREMENTS_MODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include "manual_measurement_record.h"

class ManualMeasurementsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Columns
    {
        DateTime,
        Value,
        Unit,
        Stability,
        Label,
        ColumnCount
    };

    explicit ManualMeasurementsModel(QObject *parent = nullptr);
    ~ManualMeasurementsModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addMeasurement(const ManualMeasurementRecord& record);
    void removeMeasurements(const QModelIndexList& indexes);
    void clearAll();
    void assignLabelToIndexes(const QModelIndexList& indexes, const QString& label);
    QList<ManualMeasurementRecord> getAllMeasurements() const;
    void setGroupBy(int columnEnum);

    void refresh();

private:
    struct TreeItem
    {
        QString groupKey;
        QVector<ManualMeasurementRecord> measurements;
        QVector<TreeItem*> children;
        TreeItem* parent;

        TreeItem(const QString& key = QString(), TreeItem* parentItem = nullptr)
            : groupKey(key), parent(parentItem)
        {

        }
        ~TreeItem()
            { qDeleteAll(children); }
    };

    void buildTree();
    void clearTree();
    TreeItem* getItem(const QModelIndex& index) const;
    QString getGroupKey(const ManualMeasurementRecord& record, int columnEnum) const;

    TreeItem* m_rootItem;
    QVector<ManualMeasurementRecord> m_measurements;
    int m_groupByColumn;
};

#endif // MANUAL_MEASUREMENTS_MODEL_H
