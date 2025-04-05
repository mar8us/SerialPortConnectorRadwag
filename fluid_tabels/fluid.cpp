#include "fluid.h"
#include <algorithm>

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

void Fluid::setDensityTable(const QVector<DensityPoint> &newDensityTable)
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
