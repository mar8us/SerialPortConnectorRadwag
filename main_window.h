#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "fluid_tabels/fluid_manager.h"
#include "material_tabels/material_manager.h"
#include "sample/sample_manager.h"
#include "settings/device.h"
#include "settings/device_connector.h"
#include "settings/devices_list_model.h"
#include "settings/devices_list_controler.h"

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

private slots:
    void onAddDeviceButtonClicked();
    void onEditDeviceButtonClicked();
    void onRemoveDeviceButtonClicked();
    void onDeviceComboSelectionChanged();
    void onConnectDeviceClicked();
    void onDisconnectDeviceClicked();
    void onConnectResult(bool connected);

    void onMeasurementTypeChanged();
    void onSampleComboBoxChanged(int index);

    void navigateToToolBoxPage(QWidget* page);
    void goToPreviousMeasureStage();
    void goToNextMeasureStage();
    void onMainPageChanged(int index);

    void buttonTableFluidsOnClicked();
    void buttonSamplesOnClicked();

    void onMaterialsChanged(const QMap<QString, Material> &materials);


private:
    bool canEditDevice(std::shared_ptr<const Device> &device);
    std::shared_ptr<const Device> getSelectedDevice();

    void initControls();
    void connectButtons();
    void connectMainNavButtons();
    void connectNavMeasurementButtons();
    void connectInitialDataPageButtons();
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
    void upadteSampleEditors();
    void clearSampleEditors();


    Ui::MainWindow *ui;
    DeviceListModel devicesListModel;
    DeviceControler devicesListControler;
    DeviceConnector deviceConnector;
    std::unique_ptr<FluidManager> fluidManager;
    std::unique_ptr<MaterialManager> materialManager;
    std::unique_ptr<SampleManager> sampleManager;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
};
#endif // MAIN_WINDOW_H
