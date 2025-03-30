#include "sample.h"

Sample::Sample()
    : materialDensity(0.0)
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const QString &number, const QString &name,
               const QString &materialName, const QString &materialCategory,
               const QString &materialDescription, double materialDensity,
               const QString &description, const QString &author)
    : number(number)
    , name(name)
    , materialName(materialName)
    , materialCategory(materialCategory)
    , materialDescription(materialDescription)
    , materialDensity(materialDensity)
    , description(description)
    , author(author)
{
    date = QDateTime::currentDateTime();
}

QString Sample::getNumber() const
{
    return number;
}

QString Sample::getName() const
{
    return name;
}

QString Sample::getDescription() const
{
    return description;
}

QString Sample::getAuthor() const
{
    return author;
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

void Sample::setNumber(const QString &newNumber)
{
    number = newNumber;
}

void Sample::setName(const QString &newName)
{
    name = newName;
}

void Sample::setDescription(const QString &newDescription)
{
    description = newDescription;
}

void Sample::setAuthor(const QString &newAuthor)
{
    author = newAuthor;
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
