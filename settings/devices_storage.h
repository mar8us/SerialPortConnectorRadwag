#ifndef DEVICES_STORAGE_H
#define DEVICES_STORAGE_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include "device.h"

class DeviceStorage
{
public:
    QString getStoragePath();
    bool saveDevicesToFile(const QList<std::shared_ptr<const Device>> &devices);
    QList<std::shared_ptr<const Device>> loadDevicesFromFile();

private:
    QList<DeviceCommand> parseCommands(const QJsonObject& obj);
};

#endif // DEVICES_STORAGE_H
