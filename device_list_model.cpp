#include "device_list_model.h"
#include "device_storage.h"


DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{
    devices = DeviceStorage::loadFromFile();
    for(auto &device : devices)
        deviceHash.insert(device->getGuid(), device);
}

DeviceListModel::~DeviceListModel()
{
    for(auto &device : devices)
        delete device;
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

bool DeviceListModel::updateDevicesList(const Device* device)
{

    QString deviceGuid = device->getGuid();

    auto it = deviceHash.constFind(deviceGuid);
    if(it == deviceHash.constEnd())
    {

    }
    else
    {
        int row = devices.indexOf(it.value());
        if(row != -1)
        {
            delete it.value();
            devices[row] = device;
            deviceHash[deviceGuid] = device;

            QModelIndex index = createIndex(row, 0);
            emit dataChanged(index, index);
        }
    }
    //TO DO: optimalization
    DeviceStorage::saveToFile(devices);
    return true;
}

bool DeviceListModel::addDevice(const Device* device)
{
    if(!device)
        return false;
    beginInsertRows(QModelIndex(), devices.size(), devices.size());
    devices.append(device);
    auto result = deviceHash.insert(device->getGuid(), device);
    endInsertRows();
    DeviceStorage::saveToFile(devices);
    return result.value();
}

bool DeviceListModel::editDevice(int row)
{
    if(removeDevice(row))
        return false;

    // devices[row] = device;
    // deviceHash[deviceGuid] = device;

    // QModelIndex index = createIndex(row, 0);
    // emit dataChanged(index, index);
}

bool DeviceListModel::removeDevice(int row)
{
    if(row < 0 || row >= devices.size())
        return false;

    const Device *device = getDevice(row);
    deviceHash.remove(device->getGuid());
    beginRemoveRows(QModelIndex(), row, row);
    devices.removeAt(row);
    endRemoveRows();
    delete device;

    DeviceStorage::saveToFile(devices);
    return true;
}

const Device* DeviceListModel::getDevice(int row)
{
    if(row >= 0 && row < devices.size())
        return devices[row];
    return nullptr;
}

