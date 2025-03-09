#include "device_connector.h"

DeviceConnector::DeviceConnector()
{

}

std::shared_ptr<const Device> DeviceConnector::getActiveDevice()
{
    return activeDevice;
}

bool DeviceConnector::setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice)
{
    if(!newActiveDevice || activeDevice == newActiveDevice)
        return false;
    activeDevice = newActiveDevice;
    return true;
}
