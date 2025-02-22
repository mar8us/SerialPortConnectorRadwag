#include "device.h"

Device::Device(const QString &name,
               QSerialPort::BaudRate baudRate,
               QSerialPort::DataBits dataBits,
               QSerialPort::Parity parity,
               QSerialPort::StopBits stopBits,
               const QList<DeviceCommand>& deviceCommands,
               const std::optional<QUuid> guid)
    : name(name)
    , baudRate(baudRate)
    , dataBits(dataBits)
    , parity(parity)
    , stopBits(stopBits)
    , commands(deviceCommands)
    , guid(guid.value_or(QUuid::createUuid()))
{

}

Device::Device(const Device& device)
    : name(device.name)
    , baudRate(device.baudRate)
    , dataBits(device.dataBits)
    , parity(device.parity)
    , stopBits(device.stopBits)
    , commands(device.commands)
    , guid(device.guid)
{

}

QString Device::getName() const
{
    return name;
}

QSerialPort::BaudRate Device::getBaudRate() const
{
    return baudRate;
}

QSerialPort::DataBits Device::getDataBits() const
{
    return dataBits;
}

QSerialPort::Parity Device::getParity() const
{
    return parity;
}

QSerialPort::StopBits Device::getStopBits() const
{
    return stopBits;
}

const QList<DeviceCommand>& Device::getCommands() const
{
    return commands;
}

QString Device::getGuid() const
{
    return guid.toString(QUuid::WithoutBraces);
}

void Device::setName(const QString& value)
{
    name = value;
}

void Device::setBaudRate(const QSerialPort::BaudRate value)
{
    baudRate = value;
}

void Device::setDataBits(const QSerialPort::DataBits value)
{
    dataBits = value;
}

void Device::setParity(const QSerialPort::Parity value)
{
    parity = value;
}

void Device::setStopBits(const QSerialPort::StopBits value)
{
    stopBits = value;
}

void Device::setCommands(const QList<DeviceCommand>& newCommands)
{
    commands = newCommands;
}

void Device::setGuid(const QString& newGuid)
{
    this->guid = QUuid::fromString(newGuid);
}
