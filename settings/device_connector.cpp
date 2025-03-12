#include "device_connector.h"
#include "../radwag/radwag_measure.h"

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

