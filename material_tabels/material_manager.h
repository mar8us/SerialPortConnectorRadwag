#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "material.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>

class MaterialManager : public QObject
{
    Q_OBJECT

public:
    explicit MaterialManager(QObject *parent = nullptr);

    const QMap<QString, Material> &getMaterials() const;
    void setMaterials(const QMap<QString, Material> &newMaterials);

    Material getMaterial(const QString &name) const;
    bool materialExists(const QString &name) const;
    bool reloadMaterials();

signals:

private:
    QMap<QString, Material> materials;

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    void loadMaterials();
    bool saveMaterials();
    QString getMaterialsFilePath() const;
};

#endif // MATERIALMANAGER_H
