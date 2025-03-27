#ifndef DEVICE_CONNECTOR_H
#define DEVICE_CONNECTOR_H

#include "device.h"
#include "QSerialPortInfo"
#include "../serialPort/serial_port.h"

class DeviceConnector : public QObject
{
    Q_OBJECT
public:
    DeviceConnector();

    bool connectDevice(const QString &portName);
    bool closeActiveConnection();
    bool connectionIsActive();

    std::shared_ptr<const Device> getActiveDevice();
    void setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice);
    QStringList getAvaiablePorts();
    void sendImmediateWeightCommand();

private:
    std::shared_ptr<const Device> activeDevice;
    std::unique_ptr<SerialPort> serialPort;

private slots:
    void dataReceived(const QByteArray &deviceData);

signals:
    void connectionResult(bool connected);
};

#endif // DEVICE_CONNECTOR_H
