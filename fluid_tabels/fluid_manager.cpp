#include "fluid_manager.h"
#include <QDebug>

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
    QSettings settings;
    return settings.value("FluidTablesPath", QCoreApplication::applicationDirPath() + "/fluids.json").toString();
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
    for(int i = 0; i < fluidsArray.size(); i++)
    {
        QJsonObject fluidObj = fluidsArray[i].toObject();

        // Pobranie podstawowych informacji o cieczy
        QString name = fluidObj["name"].toString();
        QString description = fluidObj["description"].toString();

        // Pobranie tabeli gęstości
        QMap<double, double> densityTable;
        QJsonArray densityArray = fluidObj["densityTable"].toArray();

        for(int j = 0; j < densityArray.size(); j++)
        {
            QJsonObject pointObj = densityArray[j].toObject();
            double temperature = pointObj["temperature"].toDouble();
            double density = pointObj["density"].toDouble();
            densityTable[temperature] = density;
        }

        Fluid fluid(name, description, densityTable);
        fluids[name] = fluid;
    }
}

bool FluidManager::saveFluids()
{
    QJsonArray fluidsArray;
    for (auto it = fluids.begin(); it != fluids.end(); ++it)
    {
        Fluid fluid = it.value();
        QJsonObject fluidObj;
        fluidObj["name"] = fluid.getName();
        fluidObj["description"] = fluid.getDescription();

        // Zapisanie tabeli gęstości jako tablicy punktów
        QJsonArray densityArray;
        QMap<double, double> densityTable = fluid.getDensityTableMap();
        QMapIterator<double, double> i(densityTable);

        while(i.hasNext())
        {
            i.next();
            QJsonObject pointObj;
            pointObj["temperature"] = i.key();
            pointObj["density"] = i.value();
            densityArray.append(pointObj);
        }

        fluidObj["densityTable"] = densityArray;
        fluidsArray.append(fluidObj);
    }

    QJsonDocument doc(fluidsArray);
    QString filePath = getFluidsFilePath();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
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
