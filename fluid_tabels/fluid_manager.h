#ifndef FLUID_MANAGER_H
#define FLUID_MANAGER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QCoreApplication>
#include "fluid.h"

class FluidManager : public QObject
{
    Q_OBJECT

public:
    explicit FluidManager(QObject *parent = nullptr);

    const QMap<QString, Fluid> &getFluids() const;
    void setFluids(const QMap<QString, Fluid> &newFluids);

    Fluid getFluid(const QString &name) const;
    bool fluidExists(const QString &name) const;
    QString getFluidsFilePath() const;

    bool reloadFluids();

private:
    QMap<QString, Fluid> fluids;

    void loadFluids();
    bool saveFluids();
};

#endif // FLUID_MANAGER_H
