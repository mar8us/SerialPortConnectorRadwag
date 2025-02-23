#ifndef DEVICE_LIST_MODEL_H
#define DEVICE_LIST_MODEL_H

#include "device.h"
#include <QObject>
#include <qabstractitemmodel.h>

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QList<std::shared_ptr<const Device>> devices;

public:

    enum class OperationResult
    {
        NullDevice,
        DeviceAdded,
        DeviceNameIsEmpty,
        DeviceNameIsNotUnique,
    };

    DeviceListModel(QObject *parent = nullptr);

    OperationResult addDevice(const std::shared_ptr<Device> &newDevice);

    bool isUniqueDeviceName(const QString &name);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // DEVICE_LIST_MODEL_H
