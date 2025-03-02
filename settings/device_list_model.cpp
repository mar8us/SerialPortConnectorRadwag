#include "device_list_model.h"

DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{

}

DeviceListModel::OperationResult DeviceListModel::addDevice(const std::shared_ptr<const Device> &newDevice)
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

DeviceListModel::OperationResult DeviceListModel::editDevice(const QString &originalDeviceName, const std::shared_ptr<const Device> &editedDevice)
{
    if(!editedDevice)
        return OperationResult::NullDevice;

    if(isUniqueDeviceName(originalDeviceName))
        return OperationResult::DeviceNotFound;

    const QString &editedDeviceName = editedDevice->getName();
    if(editedDeviceName.isEmpty())
        return OperationResult::DeviceNameIsEmpty;

    if(originalDeviceName != editedDeviceName && !isUniqueDeviceName(editedDeviceName))
        return OperationResult::DeviceNameIsNotUnique;

    int index = devicesIndexMap[originalDeviceName];
    devices.replace(index, editedDevice);
    devicesIndexMap.remove(originalDeviceName);
    devicesIndexMap.insert(editedDevice->getName(), index);

    return OperationResult::DeviceEdited;
}

const std::shared_ptr<const Device> DeviceListModel::getDevice(const QString &name) const
{
    int index = getDeviceIndex(name);
    return index != -1 ? devices[index] : nullptr;
}

const std::shared_ptr<const Device> DeviceListModel::getDevice(int row) const
{
    if(row >= 0 && row < devices.size())
        return devices.at(row);
    return nullptr;
}

int DeviceListModel::getDeviceIndex(const QString &name) const
{
    return !isUniqueDeviceName(name) ? devicesIndexMap[name] : -1;
}

bool DeviceListModel::isUniqueDeviceName(const QString &name) const
{
    return devicesIndexMap.find(name) == devicesIndexMap.end();
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    return devices.size();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= devices.size())
        return QVariant();

    const auto &device = devices.at(index.row());

    switch(role) {
    case Qt::DisplayRole:
        return device->getName();
    case Qt::ToolTipRole:
        return QString("Prędkość transmisji: %1, Bity danych: %2, Kontrola parzystości: %3, Bity stopu: %4")
            .arg(QString::number(device->getBaudRate()))
            .arg(QString::number(device->getDataBits()))
            .arg(device->getParityString())
            .arg(QString::number(device->getStopBits()));
    }

    return QVariant();
}
