#include "device.h"

Device::Device(const QString& name, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QList<DeviceCommand>& deviceCommands, const QUuid& guid)
    : name(name)
    , baudRate(baudRate)
    , dataBits(dataBits)
    , parity(parity)
    , stopBits(stopBits)
    , commands(deviceCommands)
    , guid(QUuid::createUuid())
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

QString Device::getParityString() const
{
    switch (parity)
    {
    case QSerialPort::Parity::NoParity:
        return "brak";
    case QSerialPort::Parity::EvenParity:
        return "parzysta";
    case QSerialPort::Parity::OddParity:
        return "nieparzysta";
    case QSerialPort::Parity::SpaceParity:
        return "spacja";
    case QSerialPort::Parity::MarkParity:
        return "znacznik";
    default:
        return "nieznana";
    }
}

QSerialPort::Parity Device::getParityFromString(const QString& value)
{
    if (value == "brak")
        return QSerialPort::Parity::NoParity;
    else if (value == "parzysta")
        return QSerialPort::Parity::EvenParity;
    else if (value == "nieparzysta")
        return QSerialPort::Parity::OddParity;
    else if (value == "spacja")
        return QSerialPort::Parity::SpaceParity;
    else if (value == "znacznik")
        return QSerialPort::Parity::MarkParity;
    else
        return QSerialPort::Parity::NoParity;
}

const QList<DeviceCommand>& Device::getCommands() const
{
    return commands;
}

QString Device::getGuid() const
{
    return guid.toString();
}

void Device::setName(const QString& value)
{
    name = value;
}

void Device::setBaudRate(QSerialPort::BaudRate value)
{
    baudRate = value;
}

void Device::setDataBits(QSerialPort::DataBits value)
{
    dataBits = value;
}

void Device::setParity(QSerialPort::Parity value)
{
    parity = value;
}

void Device::setStopBits(QSerialPort::StopBits value)
{
    stopBits = value;
}

void Device::setParityFromString(const QString& value)
{
    parity = getParityFromString(value);
}

void Device::setCommands(const QList<DeviceCommand>& newCommands)
{
    commands = newCommands;
}

void Device::setGuid(QString& newGuid)
{
    this->guid = QUuid::fromString(newGuid);
}
