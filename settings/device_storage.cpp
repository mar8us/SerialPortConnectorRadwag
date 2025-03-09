#include "device_storage.h"

QString DeviceStorage::getStoragePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);

    if(!dir.exists() || !dir.mkpath("."))
    {
        QMessageBox::critical(nullptr, "Błąd ścieżki", "Nie udało się utworzyć katalogu: " + path);
        return QString();
    }
    return path + "/devices.json";
}

bool DeviceStorage::saveDevicesToFile(const QList<std::shared_ptr<const Device>> &devices)
{
    QFile file(getStoragePath());

    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(nullptr, "Błąd zapisu", "Nie można zapisać danych. Brak uprawnień.");
        return false;
    }

    QJsonArray jsonArray;
    for(const auto& device : devices)
    {
        QJsonObject obj;
        obj["name"] = device->getName();
        obj["baudRate"] = device->getBaudRate();
        obj["dataBits"] = device->getDataBits();
        obj["parity"] = device->getParityString();
        obj["stopBits"] = device->getStopBits();
        obj["guid"] = device->getGuid();

        QJsonArray commandArray;
        for(const auto& command : device->getCommands())
        {
            QJsonObject commandObj;
            commandObj["description"] = command.description;
            commandObj["command"] = command.command;
            commandArray.append(commandObj);
        }

        obj["commands"] = commandArray;
        jsonArray.append(obj);
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson());
    file.close();
    return true;
}

QList<std::shared_ptr<const Device>> DeviceStorage::loadDevicesFromFile()
{
    QFile file(getStoragePath());
    QList<std::shared_ptr<const Device>> devices;

    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Błąd odczytu", "Nie można otworzyć pliku z danymi.");
        return devices;
    }

    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if(!doc.isArray())
    {
        QMessageBox::warning(nullptr, "Błąd danych", "Nieprawidłowy format danych w pliku.");
        return devices;
    }

    QJsonArray jsonArray = doc.array();
    for(const auto& value : jsonArray)
    {
        if(!value.isObject())
            continue;

        QJsonObject obj = value.toObject();
        QList<DeviceCommand> commands = parseCommands(obj);
        QString name = obj["name"].toString();
        QSerialPort::BaudRate baudRate = static_cast<QSerialPort::BaudRate>(obj["baudRate"].toInt());
        QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(obj["dataBits"].toInt());
        QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(obj["parity"].toInt());
        QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(obj["stopBits"].toInt());

        devices.append(std::make_shared<Device>(name, baudRate, dataBits, parity, stopBits, commands));
    }
    return devices;
}

QList<DeviceCommand> DeviceStorage::parseCommands(const QJsonObject& obj)
{
    QList<DeviceCommand> commands;

    if(!obj.contains("commands") || !obj["commands"].isArray())
        return commands;

    QJsonArray commandArray = obj["commands"].toArray();
    for (const auto& commandValue : commandArray)
    {
        if (!commandValue.isObject())
            continue;

        QJsonObject commandObj = commandValue.toObject();
        DeviceCommand command(commandObj["description"].toString(), commandObj["command"].toString());
        commands.append(command);
    }
    return commands;
}
