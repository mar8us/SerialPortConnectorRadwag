#ifndef MEASUREMENT_PROCESS_UI_HANDLER_H
#define MEASUREMENT_PROCESS_UI_HANDLER_H

#include <QObject>

#include "data_holder/hydrostatic_data_holder.h"
#include "../../main_widow/measurement_controller.h"
#include "../../charts/porosity_chart.h"
#include "../../charts/denisty_chart.h"
#include "measurement_state_machine.h"

#include "../../state_machine.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class MeasurementProcessUiHandler : public QObject, StageHandler<MeasurementStages::Stage>
{
    Q_OBJECT
public:
    explicit MeasurementProcessUiHandler(MainWindow *mainWindow, MeasurementController &radwagMeasureControler, HydrostaticDataHolder &dataHolder, QObject *parent = nullptr);
    virtual ~MeasurementProcessUiHandler() = default;

    void initialize();
    bool canStartMeasureProcces();

public slots:
    void onNewMeasure();
    void onReplyMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    void onContinueMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);

signals:
    void exportMeasuresExcel(const QList<const Measurement*>& measures);

private slots:
    void initializeStateMachine();

    bool onEnterStage(MeasurementStages::Stage stage, MeasurementStages::Stage) override;
    void onStartMeasurePageEnter();
    void onInitialDataPageEnter();
    void onDryMeasurePageEnter();
    void onPrepareSecondPageEnter();
    void onPrepareTriplePageEnter();
    void onFinishSecondPageEnter();
    void onSaturationMassPageEnter();
    void onFinishTriplePageEnter();
    void onSummarySecondPageEnter();
    void onSummaryTriplePageEnter();

    bool onExitStage(MeasurementStages::Stage stage, MeasurementStages::Stage toStage) override;
    bool onStartMeasurePageDataExit();
    bool onInitialDataPageExit();
    bool onDryMeasureDataPageExit();
    bool onPrepareSecondMeasurePageDataExit();
    bool onFinishSecondMeasurePageDataExit();
    bool onPrepareTripleMeasurePageDataExit();
    bool onSaturationMassMeasurePageDataExit();
    bool onFinishTripleMeasurePageDataExit();
    bool onSummarySecondPageExit();
    bool onSummaryTriplePageExit();

    void goToNextMeasureStage();
    void goToPreviousMeasureStage();
    void onStageChanged();

    void buttonTableFluidsOnClicked();
    void onButtonSeriesOnClicked();
    void onMaterialsChanged(const QMap<QString, Material> &materials);

    void onShowHydroSetSchemeButtonClicked();

    void onResetMeasureButtonClicked();
    void onSaveMeasureButtonClicekd();

    void onMeasurementTypeChanged();
    void onSampleComboBoxChanged(int index);
    void onFluidComboBoxChanged(int index);
    void onAuthorEditingFinished();

    void onComboFluidTempPrepMeasureSecondChanged(int index);
    void onComboFluidTempPrepMeasureTripleChanged(int index);
    void onComboSatMethodPrepareMeasureTripleChanged(int index);

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

    void onNewMeasureSummaryButtonClicked();
    void onReplyMeasureSummaryButtonClicked();
    void onButtonExportExcelCilcked();

    void onTextEditSecondMeasureCommentsChanged();
    void onTextEditTripleMeasureCommentsChanged();
    void onButtonSaveCommentSecondClicked();
    void onButtonSaveCommentTripleClicked();

    void onConfrimPrepareWorkstationButtonClicked();
    void onConfrimPrepareMeasureSecondButtonClicked();

    void onConfirmSampleSaturationPreparationClicked();
    void onSpinSaturationTimeChanged();

private:
    bool validateInitialDataPage();
    bool validateDryMeasureDataPage();
    bool validatePrepareSecondMeasurePage();
    bool validateFinishSecondMeasurePage();
    bool validatePrepareTripleMeasurePage();
    bool validateSaturationMassMeasurePage();
    bool validateFinishTripleMeasurePage();

    void fillSampleCombo(bool preserveSelection = false);
    void fillFluidCombo();
    void fillSaturationMethodsCombo();

    void fillPrepareMeasureTemperatureCombo();
    void fillPrepareSaturationTemperatureCombo();

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

    void updateSaveCommentSecondButtonState();
    void updateSaveCommentTripleButtonState();

    void updateSaturationMethodPrepareTriple();
    void updatePrepareSaturationFluidDensityLabel();

    void updateWigdetVisibility(MeasurementStages::Stage currentStage);

    void setupCharts();
    void updatePorosityChart(const MeasurementResults &results);
    void updateDenistyChart(const MeasurementResults &results);

    bool checkGuidePrepareWorkstation(bool showMessage = true);
    bool checkGuidePrepareMeasureSecondButton();
    bool checkGuideSampleSaturationPreparation();

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

    void initializeMappings();

    QMap<MeasurementStages::Stage, QWidget*> stageToPageMap;
    QMap<MeasurementStages::Stage, QLabel*> tripleStageToLabelMap;
    QMap<MeasurementStages::Stage, QLabel*> secondStageToLabelMap;
    QList<QLabel*> allTripleLabels;
    QList<QLabel*> allSecondLabels;


    MainWindow *mainWindow;
    Ui::MainWindow *ui;
    HydrostaticDataHolder &dataHolder;

    MeasurementController &radwagMeasureControler;
    MeasurementStateMachine *measureStateMachine;

    PorosityChartWidget *porosityChart;
    DensityChartWidget *denistyChart;
    DensityChartWidget *denistyChartSecond;

    bool saveStageData(MeasurementStages::Stage stage);
    bool saveDataToStage(MeasurementStages::Stage currentStage);
};

#endif
