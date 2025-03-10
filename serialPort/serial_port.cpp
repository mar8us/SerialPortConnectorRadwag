#include "serial_port.h"

SerialPort::SerialPort(QObject *parent)
    : QObject{parent},
    serial_port(nullptr)
{

}

SerialPort::~SerialPort()
{
    if(serial_port)
    {
        serial_port->close();
        delete serial_port;
    }
}

bool SerialPort::connect(QString portName, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    closeConnection();
    serial_port = new QSerialPort(this);
    serial_port->setPortName(portName);
    serial_port->setBaudRate(baudRate);
    serial_port->setDataBits(dataBits);
    serial_port->setParity(parity);
    serial_port->setStopBits(stopBits);

    if(serial_port->open(QIODevice::ReadWrite))
        QObject::connect(serial_port, &QSerialPort::readyRead, this, &SerialPort::dataReady);

    return isOpen();
}

bool SerialPort::closeConnection()
{
    if(serial_port == nullptr)
        return false;
    serial_port->close();
    delete serial_port;
    return true;
}

qint64 SerialPort::write(QByteArray data)
{
    if(serial_port == nullptr || !serial_port->isOpen())
        return -1;
    return serial_port->write(data);
}

void SerialPort::dataReady()
{
    if(serial_port->isOpen())
        emit dataRecevied(serial_port->readAll());
}

bool SerialPort::isOpen()
{
    return serial_port->isOpen();
}
