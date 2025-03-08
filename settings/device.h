#ifndef DEVICE_H
#define DEVICE_H

#include <QString>
#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QUuid>

struct DeviceCommand
{
    DeviceCommand(const QString &description, const QString &command)
        : description(description), command(command) {}

    QString description;
    QString command;
};

class Device
{
public:
    explicit Device(const QString &name,
                    const QSerialPort::BaudRate baudRate = QSerialPort::Baud9600,
                    const QSerialPort::DataBits dataBits = QSerialPort::Data8,
                    const QSerialPort::Parity parity = QSerialPort::NoParity,
                    const QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                    const QList<DeviceCommand> &deviceCommands = {},
                    const std::optional<QUuid> guid = std::nullopt);

    Device(const Device& source);

    QString getName() const;
    QSerialPort::BaudRate getBaudRate() const;
    QSerialPort::DataBits getDataBits() const;
    QSerialPort::Parity getParity() const;
    QString getParityString() const;
    QSerialPort::StopBits getStopBits() const;
    const QList<DeviceCommand>& getCommands() const;
    QString getGuid() const;

    void setName(const QString& value);
    void setBaudRate(const QSerialPort::BaudRate value);
    void setDataBits(const QSerialPort::DataBits value);
    void setParity(const QSerialPort::Parity value);
    void setStopBits(const QSerialPort::StopBits value);
    void setCommands(const QList<DeviceCommand>& newCommands);
    void setGuid(const QString& newGuid);

private:
    QString name;
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QList<DeviceCommand> commands;
    QUuid guid;
};

// Q_DECLARE_METATYPE(std::shared_ptr<const Device>)

#endif // DEVICE_H
