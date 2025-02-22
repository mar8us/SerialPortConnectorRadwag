#ifndef DEVICE_H
#define DEVICE_H

#include <QString>
#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QUuid>

struct DeviceCommand
{
    DeviceCommand(const QString &description, const QString &command)
        : description(description), command(command)
    {

    }

    QString description;
    QString command;
};

class Device
{
public:
    Device(const QString& name, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QList<DeviceCommand>& deviceCommands, const QUuid& guid = QUuid());
    Device(const Device& source);

    QString getName() const;
    QSerialPort::BaudRate getBaudRate() const;
    QSerialPort::DataBits getDataBits() const;
    QSerialPort::Parity getParity() const;
    QSerialPort::StopBits getStopBits() const;
    QString getParityString() const;
    static QSerialPort::Parity getParityFromString(const QString& value);
    const QList<DeviceCommand>& getCommands() const;
    QString getGuid() const;

    void setName(const QString& value);
    void setBaudRate(QSerialPort::BaudRate value);
    void setDataBits(QSerialPort::DataBits value);
    void setParity(QSerialPort::Parity value);
    void setStopBits(QSerialPort::StopBits value);
    void setCommands(const QList<DeviceCommand>& newCommands);
    void setParityFromString(const QString& value);
    void setGuid(QString& newGuid);

private:
    QString name;
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QList<DeviceCommand> commands;
    QUuid guid;
};

#endif // DEVICE_H
