#ifndef DEVICE_H
#define DEVICE_H

#include <QString>
#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QUuid>

enum class DeviceType
{
    None,
    RadwagScaleAC220,
    RadwagScaleAC350,
    Own
};

struct DeviceCommand
{
    DeviceCommand(const QString &description, const QString &command)
        : description(description), command(command) {}

    QString description;
    QString command;
};

static const QList<DeviceCommand> radwagCommands =
{
    DeviceCommand("Tarowanie", "T\r\n"),
    DeviceCommand("Zerowanie", "Z\r\n"),
    DeviceCommand("Pomiar natychmiastowy", "SI\r\n"),
    DeviceCommand("Pomiar stabilny", "S\r\n"),
    DeviceCommand("Pomiar stabilny w aktualnej jednostce", "SU\r\n"),
    DeviceCommand("Pomiar natychmiastowy w aktualnej jednostce", "SUI\r\n"),
    DeviceCommand("Rozpocznij transmisję ciągłą", "C1\r\n"),
    DeviceCommand("Zatrzymaj transmisję ciągłą", "C0\r\n"),
    DeviceCommand("Rozpocznij transmisję ciągłą w aktualnej jednostce", "CU1\r\n"),
    DeviceCommand("Zatrzymaj transmisję ciągłą w aktualnej jednostce", "CU0\r\n")
};

class Device
{
public:
    explicit Device(DeviceType deviceType,
                    const QString &name,
                    const QSerialPort::BaudRate baudRate = QSerialPort::Baud9600,
                    const QSerialPort::DataBits dataBits = QSerialPort::Data8,
                    const QSerialPort::Parity parity = QSerialPort::NoParity,
                    const QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                    const QList<DeviceCommand> &deviceCommands = {},
                    const std::optional<QUuid> guid = std::nullopt);

    Device(const Device& source);

    DeviceType getDeviceType() const;
    QString getName() const;
    QSerialPort::BaudRate getBaudRate() const;
    QSerialPort::DataBits getDataBits() const;
    QSerialPort::Parity getParity() const;
    QString getParityString() const;
    QSerialPort::StopBits getStopBits() const;
    const QList<DeviceCommand>& getCommands() const;
    QString getGuid() const;

    void setDeviceType(DeviceType type);
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
    DeviceType deviceType;
};

// Q_DECLARE_METATYPE(std::shared_ptr<const Device>)

#endif // DEVICE_H
