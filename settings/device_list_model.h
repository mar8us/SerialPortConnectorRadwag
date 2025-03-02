#ifndef DEVICE_LIST_MODEL_H
#define DEVICE_LIST_MODEL_H

#include "device.h"
#include <QObject>
#include <qabstractitemmodel.h>

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class OperationResult
    {
        NullDevice,
        DeviceAdded,
        DeviceEdited,
        DeviceNameIsEmpty,
        DeviceNameIsNotUnique,
        DeviceNotFound,
    };

    DeviceListModel(QObject *parent = nullptr);

    OperationResult addDevice(const std::shared_ptr<const Device> &newDevice);
    OperationResult editDevice(const QString &originalDeviceName, const std::shared_ptr<const Device> &editedDevice);

    const std::shared_ptr<const Device> getDevice(const QString &name) const;
    const std::shared_ptr<const Device> getDevice(int row) const;
    int getDeviceIndex(const QString &name) const;

    bool isUniqueDeviceName(const QString &name) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QList<std::shared_ptr<const Device>> devices;
    QMap<QString, int> devicesIndexMap;
};

#endif // DEVICE_LIST_MODEL_H
