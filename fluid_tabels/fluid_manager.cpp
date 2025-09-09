#include "fluid_manager.h"
#include <QDebug>
#include <qdir.h>
#include <qstandardpaths.h>

FluidManager::FluidManager(QObject *parent) : QObject(parent)
{
    loadFluids();
}

const QMap<QString, Fluid> &FluidManager::getFluids() const
{
    return fluids;
}

void FluidManager::setFluids(const QMap<QString, Fluid> &newFluids)
{
    fluids = newFluids;
    saveFluids();
}

Fluid FluidManager::getFluid(const QString &name) const
{
    if(fluids.contains(name))
        return fluids[name];
    return Fluid();
}

bool FluidManager::fluidExists(const QString &name) const
{
    return fluids.contains(name);
}

QString FluidManager::getFluidsFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(appDataPath);
    if(!dir.exists())
        dir.mkpath(".");
    return dir.filePath("fluids.json");
}

void FluidManager::loadFluids()
{
    QString filePath = getFluidsFilePath();
    QFile file(filePath);
    fluids.clear();
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Nie można otworzyć pliku cieczy:" << filePath;
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    file.close();

    if(!doc.isArray())
    {
        qWarning() << "Nieprawidłowy format pliku cieczy. Oczekiwano tablicy JSON.";
        return;
    }

    QJsonArray fluidsArray = doc.array();
    for(const QJsonValue &fluidValue : fluidsArray)
    {
        QJsonObject fluidObj = fluidValue.toObject();
        Fluid fluid;
        fluid.fromJson(fluidObj);
        fluids[fluid.getName()] = fluid;
    }
}

bool FluidManager::saveFluids()
{
    QJsonArray fluidsArray;
    for(const Fluid &fluid : fluids)
    {
        QJsonObject fluidObj = fluid.toJson();
        fluidsArray.append(fluidObj);
    }

    QJsonDocument doc(fluidsArray);
    QString filePath = getFluidsFilePath();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Nie można zapisać cieczy do pliku:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    return true;
}

bool FluidManager::reloadFluids()
{
    loadFluids();
    return true;
}
