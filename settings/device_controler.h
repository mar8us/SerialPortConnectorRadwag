#ifndef DEVICE_CONTROLER_H
#define DEVICE_CONTROLER_H

#include "device.h"
#include "device_list_model.h"

class DeviceControler : public QObject
{
    Q_OBJECT
public:
    DeviceControler(DeviceListModel &deviceListModel, QWidget *parent);

    const std::shared_ptr<const Device> beginNew();

private:
    bool handleModelOperationResult(DeviceListModel::OperationResult result, const std::shared_ptr<const Device> &device);

    DeviceListModel &deviceListModel;
    QWidget *parent;

signals:
    void addDeviceSuccess();
    void addDeviceFailed(const QString &errorMsg);

private slots:
};

#endif // DEVICE_CONTROLER_H
