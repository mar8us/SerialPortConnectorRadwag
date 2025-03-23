#include "fluid.h"
#include <algorithm>

Fluid::Fluid()
{
}

Fluid::Fluid(const QString &name, const QString &description, const QMap<double, double> &densityMap)
    : name(name), description(description), densityTable(densityMap)
{

}

Fluid::Fluid(const QString &name, const QString &description, const QVector<DensityPoint> &densityTable)
    : name(name), description(description)
{
    setDensityTable(densityTable);
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
    QMapIterator<double, double> i(densityTable);
    while(i.hasNext())
    {
        i.next();
        densityVecotr.append(DensityPoint(i.key(), i.value()));
    }
    return densityVecotr;
}

QMap<double, double> Fluid::getDensityTableMap() const
{
    return densityTable;
}

void Fluid::setDensityTable(const QVector<DensityPoint> &newDensityTable)
{
    densityTable.clear();
    for(const DensityPoint &point : newDensityTable)
        densityTable.insert(point.temperature, point.density);
}

double Fluid::getDensity(double temperature) const
{
    return densityTable.value(temperature, 0.0);
}

bool Fluid::setDensity(double temperature, double density)
{
    if(hasDensity(temperature))
        return false;
    densityTable[temperature] = density;
    return true;
}

bool Fluid::hasDensity(double temperature) const
{
    return densityTable.contains(temperature);
}

void Fluid::removeDensity(double temperature)
{
    densityTable.remove(temperature);
}
