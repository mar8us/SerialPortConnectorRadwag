#include "fluid.h"
#include <qjsonarray.h>

Fluid::Fluid()
{
}

Fluid::Fluid(const QString &name, const QString &description, const QMap<double, double> &densityMap)
    : name(name), description(description), densityMap(densityMap)
{

}

Fluid::Fluid(const QString &name, const QString &description, const QVector<DensityPoint> &densityTable)
    : name(name), description(description)
{
    setDensityTableMap(densityTable);
}

Fluid::Fluid(const Fluid &sourceFluid)
    : name(sourceFluid.name), description(sourceFluid.description)
{
    densityMap = sourceFluid.densityMap ;
}

QString Fluid::getName() const
{
    return name;
}

void Fluid::setName(const QString &newName)
{
    name = newName;
}

QString Fluid::getDescription() const
{
    return description;
}

void Fluid::setDescription(const QString &newDescription)
{
    description = newDescription;
}

QVector<DensityPoint> Fluid::getDensityTableVector() const
{
    QVector<DensityPoint> densityVecotr;
    QMapIterator<double, double> i(densityMap);
    while(i.hasNext())
    {
        i.next();
        densityVecotr.append(DensityPoint(i.key(), i.value()));
    }
    return densityVecotr;
}

QMap<double, double> Fluid::getDensityTableMap() const
{
    return densityMap;
}

void Fluid::setDensityTableMap(const QVector<DensityPoint> &newDensityTable)
{
    densityMap.clear();
    for(const DensityPoint &point : newDensityTable)
        densityMap.insert(point.temperature, point.density);
}

double Fluid::getDensity(double temperature) const
{
    return densityMap.value(temperature, 0.0);
}

bool Fluid::addDensity(double temperature, double density)
{
    if(hasDensity(temperature))
        return false;
    densityMap[temperature] = density;
    return true;
}

bool Fluid::hasDensity(double temperature) const
{
    return densityMap.contains(temperature);
}

void Fluid::removeDensity(double temperature)
{
    densityMap.remove(temperature);
}

QJsonObject Fluid::toJson() const
{
    QJsonObject obj;
    obj["name"] = name;
    obj["description"] = description;

    QJsonArray densityArray;
    for(auto it = densityMap.constBegin(); it != densityMap.constEnd(); it++)
    {
        QJsonObject densityPoint;
        densityPoint["temperature"] = it.key();
        densityPoint["density"] = it.value();
        densityArray.append(densityPoint);
    }
    obj["densityMap"] = densityArray;

    return obj;
}

void Fluid::fromJson(const QJsonObject &json)
{
    name = json["name"].toString();
    description = json["description"].toString();

    densityMap.clear();
    if(json.contains("densityMap") && json["densityMap"].isArray())
    {
        QJsonArray densityArray = json["densityMap"].toArray();
        for(const QJsonValue &value : densityArray)
        {
            if(!value.isObject())
                continue;
            QJsonObject densityPoint = value.toObject();
            double temperature = densityPoint["temperature"].toDouble();
            double density = densityPoint["density"].toDouble();
            densityMap.insert(temperature, density);
        }
    }
}
