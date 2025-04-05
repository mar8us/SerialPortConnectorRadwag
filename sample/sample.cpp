#include "sample.h"

Sample::Sample()
    : materialDensity(0.0)
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const QString &id, const QString &name,
               const QString &materialName, const QString &materialCategory,
               const QString &materialDescription, double materialDensity,
               const QString &description)
    : id(id)
    , name(name)
    , materialName(materialName)
    , materialCategory(materialCategory)
    , materialDescription(materialDescription)
    , materialDensity(materialDensity)
    , description(description)
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const Sample &sourceSample)
    : id(sourceSample.id)
    , name(sourceSample.name)
    , materialName(sourceSample.materialName)
    , materialCategory(sourceSample.materialCategory)
    , materialDescription(sourceSample.materialDescription)
    , materialDensity(sourceSample.materialDensity)
    , description(sourceSample.description)
{
    date = QDateTime::currentDateTime();
}

QString Sample::getId() const
{
    return id;
}

QString Sample::getName() const
{
    return name;
}

QString Sample::getDescription() const
{
    return description;
}

QDateTime Sample::getDate() const
{
    return date;
}

QString Sample::getMaterialName() const
{
    return materialName;
}

QString Sample::getMaterialCategory() const
{
    return materialCategory;
}

QString Sample::getMaterialDescription() const
{
    return materialDescription;
}

double Sample::getMaterialDensity() const
{
    return materialDensity;
}

void Sample::setId(const QString &newId)
{
    id = newId;
}

void Sample::setName(const QString &newName)
{
    name = newName;
}

void Sample::setDescription(const QString &newDescription)
{
    description = newDescription;
}

void Sample::setDate(const QDateTime &newDate)
{
    date = newDate;
}

void Sample::setMaterialName(const QString &newMaterialName)
{
    materialName = newMaterialName;
}

void Sample::setMaterialCategory(const QString &newMaterialCategory)
{
    materialCategory = newMaterialCategory;
}

void Sample::setMaterialDescription(const QString &newMaterialDescription)
{
    materialDescription = newMaterialDescription;
}

void Sample::setMaterialDensity(double newMaterialDensity)
{
    materialDensity = newMaterialDensity;
}

void Sample::setMaterial(const QString &materialName, const QString &materialCategory, const QString &materialDescription, double materialDensity)
{
    this->materialName = materialName;
    this->materialCategory = materialCategory;
    this->materialDescription = materialDescription;
    this->materialDensity = materialDensity;
}

void Sample::setMaterial(const Material &material)
{
    materialName = material.getName();
    materialCategory = material.getCategory();
    materialDescription = material.getDescription();
    materialDensity = material.getDensity();
}
