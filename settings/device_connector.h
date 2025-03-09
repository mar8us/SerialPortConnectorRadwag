#ifndef DEVICE_CONNECTOR_H
#define DEVICE_CONNECTOR_H

#include "device.h"
class DeviceConnector
{
public:
    DeviceConnector();

    std::shared_ptr<const Device> getActiveDevice();
    bool setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice);

private:
    std::shared_ptr<const Device> activeDevice;
};

#endif // DEVICE_CONNECTOR_H
