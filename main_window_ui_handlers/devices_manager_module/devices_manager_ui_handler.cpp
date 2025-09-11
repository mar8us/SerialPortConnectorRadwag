#include "devices_manager_ui_handler.h"
#include "../../main_window.h"
#include "../../app_core.h"

DevicesManagerUiHandler::DevicesManagerUiHandler(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , devicesListModel(this)
    , devicesListControler(devicesListModel, mainWindow)
{

}

void DevicesManagerUiHandler::initialize()
{
    connectSignals();
    ui->devicesListView->setModel(&devicesListModel);

    fillDevicesCombo(false);
    fillSerialPortCombo();

    updateStatusConnectionLabel(false);
}

void DevicesManagerUiHandler::onAddDeviceButtonClicked()
{
    devicesListControler.beginNew();
    if(!appCore.hasConnectionWithScale())
        emit fillDevicesCombo();
}

void DevicesManagerUiHandler::onEditDeviceButtonClicked()
{
    auto selectedDeivce = getSelectedDevice();
    if(!canEditDevice(selectedDeivce))
    {
        mainWindow->showWarning("Ostrzeżenie", QString("Nie możesz edytować urządzenia z aktywnym połączeniem"));
        return;
    }
    devicesListControler.beginEdit(selectedDeivce);
    if(!appCore.hasConnectionWithScale())
        fillDevicesCombo();
}

void DevicesManagerUiHandler::onRemoveDeviceButtonClicked()
{
    auto selectedDeivce = getSelectedDevice();
    if(!canEditDevice(selectedDeivce))
    {
        mainWindow->showWarning("Ostrzeżenie", QString("Nie możesz usunąć urządzenia z aktywnym połączeniem"));
        return;
    }
    devicesListControler.beginRemove(selectedDeivce);
    if(!appCore.hasConnectionWithScale())
        fillDevicesCombo();
}

void DevicesManagerUiHandler::onConnectDeviceClicked()
{
    appCore.connectScale(ui->comboBoxSelectPort->currentText(), ui->comboBoxSelectDevice->currentData().value<std::shared_ptr<const Device>>());
}

void DevicesManagerUiHandler::onDisconnectDeviceClicked()
{
    appCore.disconnectScale();
    fillDevicesCombo(true);
}

std::shared_ptr<const Device> DevicesManagerUiHandler::getSelectedDevice()
{
    QModelIndex currentIndex = ui->devicesListView->currentIndex();
    if (!currentIndex.isValid())
        return nullptr;

    DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
    if (!model)
        return nullptr;

    return model->getDevice(currentIndex.row());
}

bool DevicesManagerUiHandler::canEditDevice(std::shared_ptr<const Device> &device)
{
    auto scaleConnector = appCore.getScaleConnector();
    return !scaleConnector || (scaleConnector && device != scaleConnector->getActiveDevice());
}

void DevicesManagerUiHandler::updateStatusConnectionLabel(bool connectionStatus)
{
    if(connectionStatus)
    {
        ui->labelEditStatusConnection->setStyleSheet("color: green; font-weight: bold;");
        ui->labelEditStatusConnection->setText("Status: <font color='#2ECC71'><b>Połączono</b></font>");
        ui->comboBoxSelectDevice->setEnabled(false);
    }
    else
    {
        ui->labelEditStatusConnection->setStyleSheet("color: red; font-weight: bold;");
        ui->labelEditStatusConnection->setText("Brak połączenia");
        ui->comboBoxSelectDevice->setEnabled(true);
    }
}

void DevicesManagerUiHandler::fillDevicesCombo(bool keepActiveDevice)
{
    DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
    if(!model)
        return;

    ui->comboBoxSelectDevice->blockSignals(true);
    const QList<std::shared_ptr<const Device>>& devicesList = model->getDevicesList();

    QString currentDeviceName;
    if(keepActiveDevice)
        currentDeviceName = ui->comboBoxSelectDevice->currentText();
    ui->comboBoxSelectDevice->clear();

    for(const auto& device : devicesList)
    {
        auto deviceType = device->getDeviceType();
        if(deviceType != DeviceType::RadwagScaleAC220 && deviceType != DeviceType::RadwagScaleAC350)
            continue;

        QVariant deviceData;
        deviceData.setValue(device);
        ui->comboBoxSelectDevice->addItem(device->getName(), deviceData);
    }
    if(currentDeviceName.isEmpty())
        ui->comboBoxSelectDevice->setCurrentIndex(-1);
    else
        ui->comboBoxSelectDevice->setCurrentText(currentDeviceName);
    ui->comboBoxSelectDevice->blockSignals(false);
}

void DevicesManagerUiHandler::fillSerialPortCombo()
{
    foreach(auto &port, QSerialPortInfo::availablePorts())
        ui->comboBoxSelectPort->addItem(port.portName());
}

void DevicesManagerUiHandler::connectSignals()
{
    connectDevicesSettingsButtons();
    connect(appCore.getScaleConnector(), &DeviceConnector::connectionResult, this, &DevicesManagerUiHandler::updateStatusConnectionLabel);
}

void DevicesManagerUiHandler::connectDevicesSettingsButtons()
{
    connect(ui->addDeviceButton, &QPushButton::clicked, this, &DevicesManagerUiHandler::onAddDeviceButtonClicked);
    connect(ui->editDeviceButton, &QPushButton::clicked, this, &DevicesManagerUiHandler::onEditDeviceButtonClicked);
    connect(ui->deleteDeviceButton, &QPushButton::clicked, this, &DevicesManagerUiHandler::onRemoveDeviceButtonClicked);

    connect(ui->buttonConnectDevice, &QPushButton::clicked, this, &DevicesManagerUiHandler::onConnectDeviceClicked);
    connect(ui->buttonDisconnectDevice, &QPushButton::clicked, this, &DevicesManagerUiHandler::onDisconnectDeviceClicked);
}
