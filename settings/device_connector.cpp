#include "device_connector.h"
#include "../radwag/radwag_measure.h"

DeviceConnector::DeviceConnector()
    : serialPort(std::make_unique<SerialPort>())
{

}
bool DeviceConnector::connectDevice(const QString &portName)
{
    if(!activeDevice.get())
        return false;

    if(!serialPort->connect(portName, activeDevice->getBaudRate(), activeDevice->getDataBits(), activeDevice->getParity(), activeDevice->getStopBits()))
    {
        connectionResult(false);
        return false;
    }

    connect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);
    emit connectionResult(true);
    return true;
}

bool DeviceConnector::closeActiveConnection()
{
    if(!serialPort)
        emit connectionResult(true);
    bool connectionClosed = serialPort->closeConnection();
    emit connectionResult(!connectionClosed);
    return connectionClosed;
}

std::shared_ptr<const Device> DeviceConnector::getActiveDevice()
{
    return activeDevice;
}

void DeviceConnector::setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice)
{
    activeDevice = newActiveDevice;
}

QStringList DeviceConnector::getAvaiablePorts()
{
    QStringList ports;
    foreach(auto&port, QSerialPortInfo::availablePorts())
        ports.append(port.portName());
    return ports;
}

#include <qmessagebox.h>
void DeviceConnector::dataReceived(const QByteArray &deviceData)
{
    RadwagMeasure data(deviceData);

    qDebug() << (data.isStable() ? "Stabilny" : "Niestabilny") + QString::number(data.getValue()) + " " +  data.getUnit();
}

void DeviceConnector::sendImmediateWeightCommand()
{
    QByteArray command = "C1\r\n";  // SI + CR (carriage return) + LF (line feed)
    qint64 bytesWritten = serialPort->write(command);

    if (bytesWritten == -1) {
        qDebug() << "Błąd: Nie można wysłać komendy do wagi";
    } else {
        qDebug() << "Wysłano komendę SI. Zapisano bajtów:" << bytesWritten;
    }
}

