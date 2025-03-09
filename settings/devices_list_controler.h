#ifndef DEVICES_LIST_CONTROLER_H
#define DEVICES_LIST_CONTROLER_H

#include "device.h"
#include "devices_list_model.h"
#include "devices_storage.h"

class DeviceControler : public QObject
{
    Q_OBJECT
public:
    DeviceControler(DeviceListModel &deviceListModel, QWidget *parent);

    const std::shared_ptr<const Device> beginNew();
    const std::shared_ptr<const Device> beginEdit(const std::shared_ptr<const Device> &device);
    bool beginRemove(const std::shared_ptr<const Device> &device);

    std::shared_ptr<const Device> getActiveDevice();
    bool setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice);

private:
    bool handleModelOperationResult(DeviceListModel::OperationResult result, const std::shared_ptr<const Device> &device);

    DeviceListModel &deviceListModel;
    std::shared_ptr<const Device> activeDevice;
    DeviceStorage deviceStorage;
    QWidget *parent;

signals:
    void operationSuccess();
    void operationFailed(const QString &errorMsg);

private slots:
    void onOperationSuccess();
};

#endif // DEVICES_LIST_CONTROLER_H
