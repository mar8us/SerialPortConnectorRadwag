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
        DeviceNameIsEmpty,
        DeviceNameIsNotUnique,
    };

    DeviceListModel(QObject *parent = nullptr);

    OperationResult addDevice(const std::shared_ptr<const Device> &newDevice);

    bool isUniqueDeviceName(const QString &name);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QList<std::shared_ptr<const Device>> devices;
    QMap<QString, int> devicesIndexMap;
};

#endif // DEVICE_LIST_MODEL_H
