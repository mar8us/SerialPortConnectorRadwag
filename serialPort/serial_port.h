#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include "../settings/device.h"

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = nullptr);

    bool connect(QString portName, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    bool closeConnection();
    bool isOpen();

    qint64 write(QByteArray data);

    ~SerialPort();
signals:
    void dataRecevied(QByteArray b);

private slots:
    void dataReady();

private:
    QSerialPort *serial_port;
};

#endif // SERIAL_PORT_H
