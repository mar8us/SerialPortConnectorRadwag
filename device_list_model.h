#ifndef DEVICE_LIST_MODEL_H
#define DEVICE_LIST_MODEL_H

#include "device.h"
#include <QObject>
#include <qabstractitemmodel.h>

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QList<const Device*> devices;
    QHash<QString, const Device*> deviceHash;

public:
    DeviceListModel(QObject *parent = nullptr);
    ~DeviceListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool updateDevicesList(const Device* device);

    bool addDevice(const Device* device);
    bool editDevice(int row);
    bool removeDevice(int row);
    const Device* getDevice(int row);
};
#endif // DEVICE_LIST_MODEL_H
