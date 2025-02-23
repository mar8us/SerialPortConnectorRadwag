#include "device_list_model.h"
#include "device_storage.h"


DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{

}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    return devices.size();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return devices.at(index.row())->getName();

    return QVariant();
}

DeviceListModel::OperationResult DeviceListModel::addDevice(const std::shared_ptr<Device> &newDevice)
{
    if(!newDevice)
        return OperationResult::NullDevice;

    const QString &deviceName = newDevice->getName();
    if(deviceName.isEmpty())
        return OperationResult::DeviceNameIsEmpty;
    if(!isUniqueDeviceName(deviceName))
        return OperationResult::DeviceNameIsNotUnique;

    beginInsertRows(QModelIndex(), devices.size(), devices.size());
    devices.append(newDevice);
    endInsertRows();

    return OperationResult::DeviceAdded;
}

bool DeviceListModel::isUniqueDeviceName(const QString &name)
{
    return std::none_of(devices.begin(), devices.end(),
                        [&name](const std::shared_ptr<const Device>& existingDevice) {
                            return existingDevice->getName() == name;
                        });
}
