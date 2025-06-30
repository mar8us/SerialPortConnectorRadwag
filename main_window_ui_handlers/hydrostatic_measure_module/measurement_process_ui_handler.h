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

private slots:
    void buttonTableFluidsOnClicked();
    void buttonSamplesOnClicked();
    void onMaterialsChanged(const QMap<QString, Material> &materials);

    void onBeginNewMeasure();
    void onStartMeasureButtonClicked();
    void onShowHydroSetSchemeButtonClicked();

    void goToPreviousMeasureStage();
    void goToNextMeasureStage();

    void onResetMeasureButtonClicked();
    void onSaveMeasureButtonClicekd();

    void onSetInitialData();
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
    bool checkGuidePrepareMeasureSecondButton();

    void onConfirmSampleSaturationPreparationClicked();
    bool checkGuideSampleSaturationPreparation();
    void onSpinSaturationTimeChanged();

private:
    bool fillInitialDataLabels();
    void fillSampleCombo();
    void fillFluidCombo();
    void fillPrepareDataLabels();
    void fillSampleInfoLabels();
    void fillFluidInfoLabels();
    void fillTemperatureComboBox();
    void fillFinishMeasureSecondLabels();

    void fillPrepareSaturationDataLabels();
    void fillSaturationSampleInfoLabels();
    void fillSaturationMethodsCombo();
    void fillPrepareSaturationFluidInfoLabels();
    void fillPrepareSaturationTemperatureCombo();
    void fillMeasureSecondLabelsSummary();

    void fillFinishMeasureTripleLabels();
    void fillAirSaturatedTripleLabels();
    void fillMeasureTripleLabelsSummary();

    void updateSaveCurrentDryMeasureButtonState();
    void updateSaveFinishSecondButtonState();
    void updateSaveFinishTripleButtonState();
    void updateSaveSaturatedTripleButtonState();

    void updateFluidDensityLabel();

    void updateSaturationMethodPrepareTriple();
    void updatePrepareSaturationFluidDensityLabel();

    void setupCharts();
    void updatePorosityChart();
    void updateDenistyChart();

    bool validateUIDataForCurrentStage(MeasurementStages::Stage currentStage);
    bool validateInitialData();
    bool vaildateDryMeasureData();
    bool vaildatePrepareSecondMeasureData();
    bool vaildateFinishSecondMeasureData();
    bool vaildatePrepareTripleMeasureData();
    bool vaildateSaturationMassMeasureData();
    bool vaildateFinishTripleMeasureData();

    void upadteSampleEditors();

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

    MainWindow *mainWindow;
    Ui::MainWindow *ui;
    HydrostaticDataHolder &dataHolder;

    MeasurementController &radwagMeasureControler;

    PorosityChartWidget *porosityChart;
    DensityChartWidget *denistyChart;
};

#endif
