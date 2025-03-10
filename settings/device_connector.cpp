#include "device_connector.h"

DeviceConnector::DeviceConnector()
    : serialPort(std::make_unique<SerialPort>())
{

}
bool DeviceConnector::connectDevice(const QString &portName)
{
    if(!activeDevice)
        return false;

    if (!serialPort->connect(portName, activeDevice->getBaudRate(), activeDevice->getDataBits(), activeDevice->getParity(), activeDevice->getStopBits()))
        return false;

    connect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);

    return true;
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

QStringList DeviceConnector::getAvaiablePorts()
{
    QStringList ports;
    foreach(auto&port, QSerialPortInfo::availablePorts())
        ports.append(port.portName());
    return ports;
}

void DeviceConnector::dataReceived(QByteArray b)
{

}

