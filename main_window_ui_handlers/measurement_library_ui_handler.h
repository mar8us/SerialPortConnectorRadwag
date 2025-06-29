#ifndef MEASUREMENT_LIBRARY_UI_HANDLER_H
#define MEASUREMENT_LIBRARY_UI_HANDLER_H

#include "hydrostatic_data_holder.h"
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include "../radwag/view/measurement_tree_model.h"
#include "../radwag/view/measurement_sort_filter_proxy_model.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class MeasurementLibraryUiHandler : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementLibraryUiHandler(MainWindow *mainWindow,  HydrostaticDataHolder &dataHolder);

    void initialize();

signals:
    void newMeasureButtonClicked();

private slots:
    void onLibrarySearchTextChanged(const QString& text);
    void onLibrarySearchInChanged(int index);
    void onLibraryGroupByChanged(int index);

    void onLibraryNewMeasureButtonClicked();

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onMeasurementDoubleClicked(const QModelIndex& index);

private:
    void setupLibraryView();
    void setupLibraryControls();
    void setupLibraryModels();
    void setupLibraryTreeView();

    void refreshLibraryView();
    void updateMeasurementCounter();
    int countVisibleItems(QAbstractItemModel* model, const QModelIndex& parent);

    void fillComboLibSearchIn();
    void fillComboLibGroupBy();

    void connectSignals();
    void connectLibraryMeasureButtons();
    void connectMeasurementTreeSignals();

    MainWindow *mainWindow;
    Ui::MainWindow *ui;
    HydrostaticDataHolder &dataHolder;

    MeasurementTreeModel* measurementModel;
    MeasurementSortFilterProxyModel* measurementProxyModel;
};

#endif
