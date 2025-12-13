#ifndef MEASUREMENT_LIBRARY_UI_HANDLER_H
#define MEASUREMENT_LIBRARY_UI_HANDLER_H

#include "data_holder/hydrostatic_data_holder.h"
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include "../../radwag/view/measurement_tree_model.h"
#include "../../radwag/view/measurement_sort_filter_proxy_model.h"
#include "library_dialogs/summary_measure_dialog.h"
#include <QPointer>
#include <QMap>
#include "../../radwag/measure_statistic_analyzer.h"
#include "../../excel/measurement_exporter.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class MeasurementLibraryUiHandler : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementLibraryUiHandler(MainWindow *mainWindow,  HydrostaticDataHolder &dataHolder, QObject *parent = nullptr);

    void initialize();
    void updateCatalogButtonsState(bool hasActiveMeasurement);

signals:
    void newMeasure();
    void replySelectedMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    void continueSelectedMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    void openSamplesCatalog();
    void openFluidsCatalog();

public slots:
    void onExportExcelMeasures(const QList<const Measurement*>& measures);
    void onExportExcelMeasuresWithAnalysis(const AnalysisResult *analysisResult);

private slots:
    void onLibrarySearchTextChanged(const QString& text);
    void onLibrarySearchInChanged(int index);
    void onLibraryGroupByChanged(int index);

    void onLibraryNewMeasureButtonClicked();
    void onLibraryReplyMeasureClicked();
    void onLibraryContinueMeasureButtonClicked();
    void onLibraryDeleteMeasureButtonClicked();
    void onLibraryShowMeasureResultButtonClicked();
    void onLibraryShowAnalysisMeasuresButtonClicked();
    void onExcelExportButtonClicked();
    void onColumnsConfigButtonClicked(bool checked);

    void onCatalogSamplesButtonClicked();
    void onCatalogFluidsButtonClicked();

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onMeasurementDoubleClicked(const QModelIndex& index);

private:
    MeasurementTreeModel::TreeItem* getSelectedItem() const;
    QList<MeasurementTreeModel::TreeItem*> getSelectedItems() const;
    std::shared_ptr<const Measurement> getSelectedMeasure() const;
    std::vector<std::shared_ptr<const Measurement>> getSelectedMeasures() const;

    QList<const Measurement*> getSelectedMeasuresList() const;

    bool tryExport(MeasurementExporter& exporter);
    bool canExportExcel(const QList<const Measurement *> &measures);
    QString getExportFilePath(const QList<const Measurement*>& measures);
    QString generateExportFileName(const QList<const Measurement*>& measures) const;
    void handleSuccessfulExport(const QString& filePath);

    void setupLibraryView();
    void setupLibraryControls();
    void setupLibraryModels();
    void setupLibraryTreeView();

    void refreshLibraryView();
    void updateMeasurementCounter();
    int countVisibleItems(QAbstractItemModel* model, const QModelIndex& parent);
    void updateButtonsState();

    void showMeasureResult(std::shared_ptr<const Measurement> sourceMeasure);

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

    QMap<std::shared_ptr<const Measurement>, QPointer<SummaryMeasureDialog>> openDialogs;
};

#endif
