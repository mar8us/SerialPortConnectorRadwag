#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "fluid_tabels/fluid_manager.h"
#include "material_tabels/material_manager.h"
#include "radwag/radwag_measure.h"
#include "radwag/radwag_scale_connector.h"
#include "sample/sample_manager.h"
#include "settings/device.h"
#include "settings/device_connector.h"
#include "settings/devices_list_model.h"
#include "settings/devices_list_controler.h"
#include "radwag/measurement_manager.h"
#include "main_widow/measurement_controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void beginNewMeasure();
    void setMeasureInitialData();

private slots:
    void onAddDeviceButtonClicked();
    void onEditDeviceButtonClicked();
    void onRemoveDeviceButtonClicked();
    void onDeviceComboSelectionChanged();
    void onConnectDeviceClicked();
    void onDisconnectDeviceClicked();
    void onConnectResult(bool connected);

    void onStartMeasureButtonClicked();
    void onShowHydroSetSchemeButtonClicked();
    void onConfrimPrepareWorkstationButtonClicked();

    void onConfrimPrepareMeasureSecondButtonClicked();
    bool checkGuidePrepareMeasureSecondButton();

    void onMeasurementTypeChanged();
    void onSampleComboBoxChanged(int index);

    void navigateToToolBoxPage(QWidget* page);
    void goToPreviousMeasureStage();
    void goToNextMeasureStage();
    void onMainPageChanged(int index);

    void buttonTableFluidsOnClicked();
    void buttonSamplesOnClicked();

    void onMaterialsChanged(const QMap<QString, Material> &materials);

    void onBeginNewMeasure();
    void onSetInitialData();

    void onGetCurrentDryMeasureButtonClicked();
    void onRadwagMeasueReady(const RadwagMeasure &data);
    void onSaveCurrentDryMeasureButtonClicked();
    void onClearSavedDryMeasureButtonClicked();

    void onDryMassExecuteStepOneClicked();
    void onDryMassExecuteStepTwoClicked();
    void onDryMassExecuteStepThreeClicked();

    void onGetCurrentFinishMeasureSecondButtonClicked();
    void onSaveCurrentFinishMeasureSecondButtonClicked();
    void onClearSavedFinishMeasureSecondButtonClicked();

    void onFinishMeasureSecondStepTwoClicked();
    void onFinishMeasureSecondStepTreeClicked();

    void onNewSecondMeasureSecondButtonClicked();
    void onReplySecondMeasureSecondButtonClicked();

private:
    bool canEditDevice(std::shared_ptr<const Device> &device);
    std::shared_ptr<const Device> getSelectedDevice();
    void connectDevice();
    void disconnectDevice();

    void initControls();
    void connectButtons();
    void connectMainNavButtons();
    void connectNavMeasurementButtons();
    void connectPrepareWorksationPageButtons();
    void connectInitialDataPageButtons();
    void connectPrepareMeasureSecondPageButtons();
    void connectDryMassPageButtons();
    void connectFinishSecondPageButtons();
    void connectSummaryMeasureSecondPageButtons();
    void connectCatalogsButtons();
    void connectDevicesSettingsButtons();
    void updateStageLabels();
    void finishMeasurement();
    void setProperty();
    void setIcons();
    void fillDevicesCombo(bool keepActiveDevice = false);
    void fillSerialPortCombo();
    void fillSampleCombo();
    void fillFluidCombo();
    void updateActionIcons(int index);
    void updateStatusConnectionLabel(bool connectionStatus);
    void updateConnectonLabelsStatusBar(bool connectionStatus);
    void upadteSampleEditors();
    void clearSampleEditors();

    // MeasurementRadwag
    bool fillInitialDataLabels();
    void fillPrepareDataLabels();
    void fillSampleInfoLabels();
    void fillFluidInfoLabels();
    void fillTemperatureComboBox();
    void updateFluidDensityLabel();
    void fillFinishMeasureSecondLabels();
    void fillMeasureSecondLabelsSummary();

    bool checkDeviceConnectionWithMessage();
    bool validateInitialData();
    bool vaildateDryMeasureData();

    void clearInitialDataPage();
    void clearDryMeasurePage();
    void clearPrepareMeasureSecondPage();
    void clearFinishMeasureSecondPage();

    void setEnableInitialDataPage(bool enabled);
    void setEnableDryMeasurePage(bool enabled);
    void setEnablePrepareMeasureSecondPage(bool enabled);
    void setEnableFinishMeasureSecondPage(bool enabled);

    void updateSaveCurrentDryMeasureButtonState();
    void updateSaveFinishSecondButtonState();

    Ui::MainWindow *ui;
    DeviceListModel devicesListModel;
    DeviceControler devicesListControler;
    std::unique_ptr<RadwagScaleConnector> radwagScaleConnector;
    std::unique_ptr<FluidManager> fluidManager;
    std::unique_ptr<MaterialManager> materialManager;
    std::unique_ptr<SampleManager> sampleManager;
    std::shared_ptr<MeasurementManager> measurememntManager;
    std::unique_ptr<MeasurementController> radwagMeasureControler;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
    static inline const QColor MEASURE_LABEL_COLOR = QColor(0, 100, 255);
};
#endif // MAIN_WINDOW_H









// #ifndef MAIN_WINDOW_H
// #define MAIN_WINDOW_H

// #include <QMainWindow>
// #include "fluid_tabels/fluid_manager.h"
// #include "material_tabels/material_manager.h"
// #include "radwag/radwag_measure.h"
// #include "sample/sample_manager.h"
// #include "settings/device.h"
// #include "settings/device_connector.h"
// #include "settings/devices_list_model.h"
// #include "settings/devices_list_controler.h"
// #include "radwag/measurement_manager.h"
// #include "main_widow/measurement_controller.h"

// QT_BEGIN_NAMESPACE
// namespace Ui {
// class MainWindow;
// }
// QT_END_NAMESPACE

// class MainWindow : public QMainWindow
// {
//     Q_OBJECT

// public:
//     MainWindow(QWidget *parent = nullptr);
//     ~MainWindow();

// signals:
//     void beginNewMeasure();

// private slots:
//     void onAddDeviceButtonClicked();
//     void onEditDeviceButtonClicked();
//     void onRemoveDeviceButtonClicked();
//     void onDeviceComboSelectionChanged();
//     void onConnectDeviceClicked();
//     void onDisconnectDeviceClicked();
//     void onConnectResult(bool connected);

//     void onStartMeasureButtonClicked();
//     void onShowHydroSetSchemeButtonClicked();
//     void onConfrimPrepareWorkstationButtonClicked();

//     void onMeasurementTypeChanged();
//     void onSampleComboBoxChanged(int index);

//     void navigateToToolBoxPage(QWidget* page);
//     void goToPreviousMeasureStage();
//     void goToNextMeasureStage();
//     void onMainPageChanged(int index);

//     void buttonTableFluidsOnClicked();
//     void buttonSamplesOnClicked();

//     void onMaterialsChanged(const QMap<QString, Material> &materials);

//     void onBeginNewMeasure();
//     void onGetCurrentDryMeasureButtonClicked();
//     void onRadwagMeasueReady(const RadwagMeasure &data);
//     void onSaveCurrentDryMeasureButtonClicked();
//     void onClearSavedDryMeasureButtonClicked();

//     void onDryMassExecuteStepOneClicked();
//     void onDryMassExecuteStepTwoClicked();
//     void onDryMassExecuteStepThreeClicked();

//     void onGetCurrentFinishMeasureSecondButtonClicked();
//     void onSaveCurrentFinishMeasureSecondButtonClicked();
//     void onClearSavedFinishMeasureSecondButtonClicked();
//     void onCalculatelabelVolumeFinishMeasureSecondButtonClicked();

//     void onFinishMeasureSecondStepTwoClicked();
//     void onFinishMeasureSecondStepTreeClicked();

// private:
//     bool canEditDevice(std::shared_ptr<const Device> &device);
//     std::shared_ptr<const Device> getSelectedDevice();

//     void initControls();
//     void connectButtons();
//     void connectMainNavButtons();
//     void connectNavMeasurementButtons();
//     void connectPrepareWorksationPageButtons();
//     void connectInitialDataPageButtons();
//     void connectDryMassPageButtons();
//     void connectFinishSecondPageButtons();
//     void connectCatalogsButtons();
//     void connectDevicesSettingsButtons();
//     void updateStageLabels();
//     void finishMeasurement();
//     void setProperty();
//     void setIcons();
//     void fillDevicesCombo(bool keepActiveDevice = false);
//     void fillSerialPortCombo();
//     void fillSampleCombo();
//     void fillFluidCombo();
//     void updateActionIcons(int index);
//     void updateStatusConnectionLabel(bool connectionStatus);
//     void updateConnectonLabelsStatusBar(bool connectionStatus);
//     void upadteSampleEditors();
//     void clearSampleEditors();

//     // MeasurementRadwag
//     void fillPrepareDataLabels();
//     void fillSampleInfoLabels();
//     void fillFluidInfoLabels();
//     void fillTemperatureComboBox();
//     void updateFluidDensityLabel();
//     void fillFinishMeasureSecondLabels();

//     bool checkDeviceConnectionWithMessage();
//     bool validateInitialData();
//     bool vaildateDryMeasureData();

//     void updateSaveCurrentDryMeasureButtonState();
//     void updateSaveFinishSecondButtonState();

//     Ui::MainWindow *ui;
//     DeviceListModel devicesListModel;
//     DeviceControler devicesListControler;
//     DeviceConnector deviceConnector;
//     std::unique_ptr<FluidManager> fluidManager;
//     std::unique_ptr<MaterialManager> materialManager;
//     std::unique_ptr<SampleManager> sampleManager;
//     std::shared_ptr<MeasurementManager> measurememntManager;
//     std::unique_ptr<MeasurementController> radwagMeasureControler;

//     QIcon defaultSettingsIcon;
//     QIcon activeSettingsIcon;
//     QIcon defaultRadwagIcon;
//     QIcon activeRadwagIcon;

//     static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
// };
// #endif // MAIN_WINDOW_H
