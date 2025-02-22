#ifndef DEVICE_STORAGE_H
#define DEVICE_STORAGE_H

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
    static QString getStoragePath();
    static bool saveToFile(const QList<const Device*>& devices);
    static QList<const Device*> loadFromFile();

private:
    static QList<DeviceCommand> parseCommands(const QJsonObject& obj);
};

#endif // DEVICE_STORAGE_H
