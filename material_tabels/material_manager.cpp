#include "material_manager.h"
#include <QDebug>

MaterialManager::MaterialManager(QObject *parent) : QObject(parent)
{
    loadMaterials();
}

const QMap<QString, Material> &MaterialManager::getMaterials() const
{
    return materials;
}

void MaterialManager::setMaterials(const QMap<QString, Material> &newMaterials)
{
    materials = newMaterials;
    saveMaterials();
}

Material MaterialManager::getMaterial(const QString &name) const
{
    if(materials.contains(name))
        return materials[name];
    return Material();
}

bool MaterialManager::materialExists(const QString &name) const
{
    return materials.contains(name);
}

QString MaterialManager::getMaterialsFilePath() const
{
    QSettings settings;
    return settings.value("MaterialTablesPath", QCoreApplication::applicationDirPath() + "/materials.json").toString();
}

void MaterialManager::loadMaterials()
{
    QString filePath = getMaterialsFilePath();
    QFile file(filePath);

    materials.clear();

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Nie można otworzyć pliku materiałów:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    file.close();

    if(!doc.isArray())
    {
        qWarning() << "Nieprawidłowy format pliku materiałów. Oczekiwano tablicy JSON.";
        return;
    }

    QJsonArray materialsArray = doc.array();
    for(int i = 0; i < materialsArray.size(); i++)
    {
        QJsonObject materialObj = materialsArray[i].toObject();
        Material material;
        material.fromJson(materialObj);
        materials[material.getName()] = material;
    }
}

bool MaterialManager::saveMaterials()
{
    QJsonArray materialsArray;

    for (auto it = materials.begin(); it != materials.end(); ++it)
    {
        Material material = it.value();
        materialsArray.append(material.toJson());
    }

    QJsonDocument doc(materialsArray);
    QString filePath = getMaterialsFilePath();

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Nie można zapisać materiałów do pliku:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    return true;
}

bool MaterialManager::reloadMaterials()
{
    loadMaterials();
    return true;
}
