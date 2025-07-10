#ifndef MEASUREMENT_PROCESS_UI_HANDLER_H
#define MEASUREMENT_PROCESS_UI_HANDLER_H

#include <QObject>

#include "data_holder/hydrostatic_data_holder.h"
#include "../../main_widow/measurement_controller.h"
#include "../../charts/porosity_chart.h"
#include "../../charts/denisty_chart.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class MeasurementProcessUiHandler : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementProcessUiHandler(MainWindow *mainWindow, MeasurementController &radwagMeasureControler, HydrostaticDataHolder &dataHolder);

    void initialize();

signals:
    void setMeasureInitialData();
    void updateConnectonLabelsStatusBar(bool status);

public slots:
    void onNewMeasure();
private slots:
    void buttonTableFluidsOnClicked();
    void buttonSamplesOnClicked();
    void onMaterialsChanged(const QMap<QString, Material> &materials);

    void onStartMeasureButtonClicked();
    void onShowHydroSetSchemeButtonClicked();

    void goToPreviousMeasureStage();
    void goToNextMeasureStage();

    void onResetMeasureButtonClicked();
    void onSaveMeasureButtonClicekd();

    void onSetMeasureInitialData();
    void onMeasurementTypeChanged();
    void onSampleComboBoxChanged(int index);

    void onDryMassExecuteStepOneClicked();
    void onDryMassExecuteStepTwoClicked();
    void onDryMassExecuteStepThreeClicked();
    void onGetCurrentDryMeasureButtonClicked();
    void onSaveCurrentDryMeasureButtonClicked();
    void onClearSavedDryMeasureButtonClicked();

    void onFinishMeasureSecondStepTwoClicked();
    void onFinishMeasureSecondStepTreeClicked();
    void onGetCurrentFinishMeasureSecondButtonClicked();
    void onSaveCurrentFinishMeasureSecondButtonClicked();
    void onClearSavedFinishMeasureSecondButtonClicked();

    void onFinishMeasureTripleStepTwoClicked();
    void onFinishMeasureTripleExecuteStepTreeClicked();
    void onGetTripleCurrentFinishMeasureButtonClicked();
    void onSaveTripleCurrentFinishMeasureButtonClicked();
    void onClearTripleSavedFinishMeasureButtonClicked();

    void onSaturatedMassTripleStepFourClicked();
    void onSaturatedMassTripleStepFiveClicked();
    void onGetTripleCurrentSaturatedMeasureButtonClicked();
    void onSaveTripleCurrentSaturatedMeasureButtonClicked();
    void onClearTripleSavedSaturatedMeasureButtonClicked();


    //Summary second
    void onSaveSecondMeasureButtonClicked();
    void onNewMeasureButtonClicked();
    void onReplyMeasureButtonClicked();


    void onConfrimPrepareWorkstationButtonClicked();
    void onConfrimPrepareMeasureSecondButtonClicked();

    void onConfirmSampleSaturationPreparationClicked();
    void onSpinSaturationTimeChanged();

private:
    void fillSampleCombo();
    void fillFluidCombo();
    void fillSaturationMethodsCombo();

    void upadteSampleEditors();

    bool updateInitialDataLabels();
    void updateDryDataLabels();
    void updatePrepareDataLabels();

    void updateSampleInfoLabels();
    void updateFluidInfoLabels();
    void updateTemperatureComboBox();
    void updateFluidDensityLabel();
    void updateFinishMeasureSecondLabels();
    void updateMeasureSecondLabelsSummary();

    void updatePrepareSaturationDataLabels();
    void updateSaturationSampleInfoLabels();
    void updatePrepareSaturationFluidInfoLabels();
    void updatePrepareSaturationTemperatureCombo();
    void updateFinishMeasureTripleLabels();
    void updateAirSaturatedTripleLabels();
    void updateMeasureTripleLabelsSummary();

    void updateSaveCurrentDryMeasureButtonState();
    void updateSaveFinishSecondButtonState();
    void updateSaveFinishTripleButtonState();
    void updateSaveSaturatedTripleButtonState();

    void updateSaturationMethodPrepareTriple();
    void updatePrepareSaturationFluidDensityLabel();

    void setupCharts();
    void updatePorosityChart();
    void updateDenistyChart();

    bool checkGuidePrepareWorkstation();
    bool checkGuidePrepareMeasureSecondButton();
    bool checkGuideSampleSaturationPreparation();

    bool validateUIDataForCurrentStage(MeasurementStages::Stage currentStage);
    bool validateInitialData();
    bool vaildateDryMeasureData();
    bool vaildatePrepareSecondMeasureData();
    bool vaildateFinishSecondMeasureData();
    bool vaildatePrepareTripleMeasureData();
    bool vaildateSaturationMassMeasureData();
    bool vaildateFinishTripleMeasureData();

    void clearSecondMeasurePages();
    void clearTripleMeasurePages();
    void clearSampleEditors();
    void clearInitialDataPage();
    void clearDryMeasurePage();
    void clearPrepareMeasureSecondPage();
    void clearFinishMeasureSecondPage();
    void clearPrepareMeasureTriplePage();
    void clearFinishMeasureTriplePage();
    void clearAirSaturatedTriplePage();

    void setEnableInitialDataPage(bool enabled);
    void setEnableDryMeasurePage(bool enabled);
    void setEnablePrepareMeasureSecondPage(bool enabled);
    void setEnableFinishMeasureSecondPage(bool enabled);
    void setEnablePrepareSaturationPage(bool enabled);
    void setEnableSaturationTrilpePage(bool enabled);
    void setEnableSaturatedTrilpePage(bool enabled);

    void connectSignals();
    void connectNavMeasurementButtons();
    void connectStagesOperationButtons();
    void connectPrepareWorksationPageButtons();
    void connectInitialDataPageButtons();
    void connectPrepareMeasureSecondPageButtons();
    void connectPrepareSaturationButton();
    void connectDryMassPageButtons();
    void connectFinishSecondPageButtons();
    void connectFinishTriplePageButtons();
    void connectSaturatedTriplePageButtons();
    void connectSummaryMeasureSecondPageButtons();
    void connectSummaryMeasureTriplePageButtons();
    void connectCatalogsButtons();

    void updateStageLabels();

    MeasurementStages::Stage getCurrentStage() const;
    void initializeMappings();

    QMap<MeasurementStages::Stage, QLabel*> tripleStageToLabelMap;
    QMap<MeasurementStages::Stage, QLabel*> secondStageToLabelMap;
    QList<QLabel*> allTripleLabels;
    QList<QLabel*> allSecondLabels;


    MainWindow *mainWindow;
    Ui::MainWindow *ui;
    HydrostaticDataHolder &dataHolder;

    MeasurementController &radwagMeasureControler;

    PorosityChartWidget *porosityChart;
    DensityChartWidget *denistyChart;
};

#endif
