#ifndef DEVICES_MANAGER_UI_HANDLER_H
#define DEVICES_MANAGER_UI_HANDLER_H

#include <QObject>
#include "../hydrostatic_measure_module/hydrostatic_measure_module.h"
#include "../../settings/devices_list_model.h"
#include "../../settings/devices_list_controler.h"

class DevicesManagerUiHandler : public QObject
{
    Q_OBJECT
public:
    explicit DevicesManagerUiHandler(MainWindow *mainWindow);
    ~DevicesManagerUiHandler() = default;

    void initialize();

private slots:
    void onAddDeviceButtonClicked();
    void onEditDeviceButtonClicked();
    void onRemoveDeviceButtonClicked();

    void onConnectDeviceClicked();
    void onDisconnectDeviceClicked();

private:
    std::shared_ptr<const Device> getSelectedDevice();
    bool canEditDevice(std::shared_ptr<const Device> &device);

    void updateStatusConnectionLabel(bool connectionStatus);

    void fillDevicesCombo(bool keepActiveDevice = true);
    void fillSerialPortCombo();

    void connectSignals();
    void connectDevicesSettingsButtons();

    DeviceListModel devicesListModel;
    DeviceControler devicesListControler;

    MainWindow *mainWindow;
    Ui::MainWindow *ui;
};

#endif
