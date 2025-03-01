#include "device_list_model.h"

DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{

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
    devicesIndexMap[newDevice->getName()] = devices.size() - 1;
    endInsertRows();

    return OperationResult::DeviceAdded;
}

bool DeviceListModel::isUniqueDeviceName(const QString &name)
{
    return devicesIndexMap.find(name) == devicesIndexMap.end();
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
