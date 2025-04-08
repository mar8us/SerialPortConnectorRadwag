#ifndef FLUID_H
#define FLUID_H

#include <QString>
#include <QVector>
#include <QMap>
#include <qjsonobject.h>

struct DensityPoint
{
    double temperature;
    double density;

    DensityPoint(double temp = 0.0, double dens = 0.0)
        : temperature(temp), density(dens) {}
};

class Fluid
{
public:
    Fluid();
    Fluid(const QString &name, const QString &description, const QMap<double, double> &densityMap);
    Fluid(const QString &name, const QString &description, const QVector<DensityPoint> &densityTable);
    Fluid(const Fluid &sourceFluid);

    QString getName() const;
    void setName(const QString &name);

    QString getDescription() const;
    void setDescription(const QString &description);

    QVector<DensityPoint> getDensityTableVector() const;
    const QMap<double, double> &getDensityTableMap() const;
    void setDensityTableMap(const QVector<DensityPoint> &densityTable);

    double getDensity(double temperature) const;
    bool addDensity(double temperature, double density);
    bool hasDensity(double temperature) const;
    void removeDensity(double temperature);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString name;
    QString description;
    QMap<double, double> densityMap;
};

#endif // FLUID_H
