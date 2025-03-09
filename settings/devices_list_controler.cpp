#include "devices_list_controler.h"
#include "device_form.h"
#include "../message_utils.h"

DeviceControler::DeviceControler(DeviceListModel &deviceListModel, QWidget *parent)
    : deviceListModel(deviceListModel), parent(parent)
{
    deviceListModel.setDevicesList(deviceStorage.loadDevicesFromFile());
    connect(this, &DeviceControler::operationSuccess, this, &DeviceControler::onOperationSuccess);
}

const std::shared_ptr<const Device> DeviceControler::beginNew()
{
    auto newDevice = std::make_shared<Device>(QString());
    auto dialog = std::make_unique<DeviceForm>(newDevice, parent);

    bool result = false;
    connect(dialog.get(), &DeviceForm::dialogAccepted, this,
            [this, &result](const std::shared_ptr<const Device> &newDevice)
            {
                result = handleModelOperationResult(deviceListModel.addDevice(newDevice), newDevice);
            });
    connect(this, &DeviceControler::operationSuccess, dialog.get(), &DeviceForm::onAddDeviceSuccess);
    connect(this, &DeviceControler::operationFailed, dialog.get(), &DeviceForm::onAddDeviceFailed);

    dialog->setWindowTitle("Dodaj urządzenie");
    dialog->exec();

    return result ? std::shared_ptr<const Device>(newDevice) : nullptr;
}

const std::shared_ptr<const Device> DeviceControler::beginEdit(const std::shared_ptr<const Device> &device)
{
    if(!device)
        return nullptr;
    auto editedDevice = std::make_shared<Device>(*device);
    auto dialog = std::make_unique<DeviceForm>(editedDevice, parent);
    bool result = false;

    connect(dialog.get(), &DeviceForm::dialogAccepted, this,
            [this, &result, device](const std::shared_ptr<Device> &editedDevice)
            {
                result = handleModelOperationResult(deviceListModel.replaceDevice(device->getName(), editedDevice), editedDevice);
            });

    connect(this, &DeviceControler::operationSuccess, dialog.get(), &DeviceForm::onAddDeviceSuccess);
    connect(this, &DeviceControler::operationFailed, dialog.get(), &DeviceForm::onAddDeviceFailed);

    dialog->setWindowTitle("Edytuj urządzenie");
    dialog->exec();

    return result ? editedDevice : nullptr;
}

bool DeviceControler::beginRemove(const std::shared_ptr<const Device> &device)
{
    if(!device)
        return false;
    if(!Messages::showYesNoWarning(parent,  QString("Czy na pewno chcesz usunąć urządzenie o nazwie: \"%1\" ?").arg(device->getName())))
        return false;
    return handleModelOperationResult(deviceListModel.removeDevice(device->getName()), NULL);
}

std::shared_ptr<const Device> DeviceControler::getActiveDevice()
{
    return activeDevice;
}

bool DeviceControler::setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice)
{
    if(!newActiveDevice || activeDevice == newActiveDevice)
        return false;
    activeDevice = newActiveDevice;
    return true;
}

bool DeviceControler::handleModelOperationResult(DeviceListModel::OperationResult result, const std::shared_ptr<const Device> &device)
{
    switch(result)
    {
        case DeviceListModel::OperationResult::DeviceAdded:
        case DeviceListModel::OperationResult::DeviceEdited:
            emit operationSuccess();
            return true;

        case DeviceListModel::OperationResult::DeviceRemoved:
            operationSuccess();
            return true;

        case DeviceListModel::OperationResult::NullDevice:
            emit operationFailed("Urządzenie jest puste!");
            return false;

        case DeviceListModel::OperationResult::DeviceNameIsEmpty:
            emit operationFailed("Nazwa urządzenia nie może być pusta!");
            return false;

        case DeviceListModel::OperationResult::DeviceNameIsNotUnique:
            emit operationFailed("Urządzenie o nazwie \"" + device->getName() + "\" już istnieje!");
            return false;

        case DeviceListModel::OperationResult::DeviceNotFound:
            emit operationFailed("Nie odnaleziono urządzenia!");
            return false;

        default:
            emit operationFailed("Nieznany błąd!");
            return false;
    }
}

void DeviceControler::onOperationSuccess()
{
    deviceStorage.saveDevicesToFile(deviceListModel.getDevicesList());
}
