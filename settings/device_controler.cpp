#include "device_controler.h"
#include "add_divice_form.h"
#include <QMessageBox>

DeviceControler::DeviceControler(DeviceListModel &deviceListModel, QWidget *parent)
    : deviceListModel(deviceListModel), parent(parent)
{

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
    connect(this, &DeviceControler::addDeviceSuccess, dialog.get(), &DeviceForm::onAddDeviceSuccess);
    connect(this, &DeviceControler::addDeviceFailed, dialog.get(), &DeviceForm::onAddDeviceFailed);

    dialog->setWindowTitle("Dodaj urządzenie");
    dialog->exec();

    return result ? std::shared_ptr<const Device>(newDevice) : nullptr;
}

void DeviceControler::endNewDevice(const std::shared_ptr<Device> &newDevice)
{
    handleModelOperationResult(deviceListModel.addDevice(newDevice), newDevice);
}

bool DeviceControler::handleModelOperationResult(DeviceListModel::OperationResult result, const std::shared_ptr<const Device> &device)
{
    switch(result)
    {
        case DeviceListModel::OperationResult::DeviceAdded:
            emit addDeviceSuccess();
            return true;

        case DeviceListModel::OperationResult::NullDevice:
            emit addDeviceFailed("Urządzenie jest puste!");
            return false;

        case DeviceListModel::OperationResult::DeviceNameIsEmpty:
            emit addDeviceFailed("Nazwa urządzenia nie może być pusta!");
            return false;

        case DeviceListModel::OperationResult::DeviceNameIsNotUnique:
            emit addDeviceFailed("Urządzenie o nazwie \"" + device->getName() + "\" już istnieje!");
            return false;

        default:
            emit addDeviceFailed("Nieznany błąd!");
            return false;
    }
}
