#ifndef MEASUREMENT_TREE_MODEL_H
#define MEASUREMENT_TREE_MODEL_H

#include <QObject>
#include <qabstractitemmodel.h>
#include "../measurement_manager.h"
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QComboBox>
#include <qapplication.h>

class MeasurementTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Columns
    {
        MeasureType,
        SampleName,
        Material,
        Fluid,
        ApparetDenisty,
        TotalPorosity,
        Status,
        Author,
        Date,
        EndDate,
        MeasureId,
        ColumnCount
    };

    explicit MeasurementTreeModel(MeasurementManager* manager, QObject* parent = nullptr);
    ~MeasurementTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setGroupBy(int groupByIndex);
    void setFilterText(const QString& text);
    void setFilterColumn(int column);
    void setMeasurementType(MeasurementType type);
    void refresh();


    QString getStatusText(MeasurementStatus status) const;
    bool matchesFilter(const std::shared_ptr<const Measurement>& measurement, int filterColumn, const QString& filterText) const;
    QString getGroupKey(const std::shared_ptr<const Measurement>& measurement, int groupByOption) const;

private slots:
    void onMeasurementChanged(const QString& id);

public:
    struct TreeItem
    {
        QString groupKey;
        QVector<QString> measurementIds;
        QVector<TreeItem*> children;
        TreeItem* parent;

        TreeItem(const QString& key = QString(), TreeItem* parentItem = nullptr)
            : groupKey(key), parent(parentItem) {}
        ~TreeItem() { qDeleteAll(children); }
    };

    void buildTree();
    void clearTree();
    TreeItem* getItem(const QModelIndex& index) const;

    MeasurementManager* measurementManager;
    TreeItem* rootItem;
    int groupByOption;
    QString filterText;
    int filterColumn;
    MeasurementType currentType;
};



// class LibraryViewController : public QObject
// {
//     Q_OBJECT
// public:
//     explicit LibraryViewController(QObject* parent = nullptr);

//     void setupTreeView(QTreeView* treeViewTwoStage, QTreeView* treeViewThreeStage,
//                        QComboBox* searchIn, QComboBox* sortBy, QComboBox* groupBy);

// public slots:
//     void onSearchTextChanged(const QString& text);
//     void onSearchInChanged(int index);
//     void onSortByChanged(int index);
//     void onGroupByChanged(int index);
//     void onMeasurementAdded(const QString& id);
//     void onMeasurementUpdated(const QString& id);
//     void onMeasurementRemoved(const QString& id);

// private:
//     MeasurementManager* measurementManager;
//     MeasurementTreeModel* twoStageModel;
//     MeasurementTreeModel* threeStageModel;
//     MeasurementSortFilterProxyModel* twoStageProxyModel;
//     MeasurementSortFilterProxyModel* threeStageProxyModel;

//     void refreshModels();
// };

#endif // MEASUREMENT_TREE_MODEL_H
