#include "device_controler.h"
#include "add_divice_form.h"
#include "../message_utils.h"

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

const std::shared_ptr<const Device> DeviceControler::beginEdit(const QString& deviceToEditName)
{
    const auto originalDevice = deviceListModel.getDevice(deviceToEditName);
    if(!originalDevice.get())
        return nullptr;

    auto editedDevice = std::make_shared<Device>(*originalDevice);
    auto dialog = std::make_unique<DeviceForm>(editedDevice, parent);

    bool result = false;
    connect(dialog.get(), &DeviceForm::dialogAccepted, this,
            [this, &result, originalName = deviceToEditName](const std::shared_ptr<Device> &editedDevice)
            {
                result = handleModelOperationResult(deviceListModel.editDevice(originalName, editedDevice), editedDevice);
            });
    connect(this, &DeviceControler::addDeviceSuccess, dialog.get(), &DeviceForm::onAddDeviceSuccess);
    connect(this, &DeviceControler::addDeviceFailed, dialog.get(), &DeviceForm::onAddDeviceFailed);

    dialog->setWindowTitle("Edytuj urządzenie");
    dialog->exec();

    return result ? std::shared_ptr<const Device>(editedDevice) : nullptr;
}

bool DeviceControler::beginRemove(const QString& deviceToEditName)
{
    auto device = deviceListModel.getDevice(deviceToEditName);
    if(!Messages::showYesNoWarning(parent,  QString("Czy na pewno chcesz usunąć urządzenie o nazwie: \"%1\" ?").arg(device->getName())))
        return false;
    return handleModelOperationResult(deviceListModel.removeDevice(device->getName()), NULL);
}

bool DeviceControler::handleModelOperationResult(DeviceListModel::OperationResult result, const std::shared_ptr<const Device> &device)
{
    switch(result)
    {
        case DeviceListModel::OperationResult::DeviceAdded:
        case DeviceListModel::OperationResult::DeviceEdited:
            emit addDeviceSuccess();
            return true;

        case DeviceListModel::OperationResult::DeviceRemoved:
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

        case DeviceListModel::OperationResult::DeviceNotFound:
            emit addDeviceFailed("Nie odnaleziono edytowanego urządzenia!");
            return false;

        default:
            emit addDeviceFailed("Nieznany błąd!");
            return false;
    }
}
