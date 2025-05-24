#include "sample.h"
#include <qjsonobject.h>

Sample::Sample()
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const QString &id, const QString &name, const QString &description, const Material &material)
    : id(id)
    , name(name)
    , description(description)
    , material(material)
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const Sample &sourceSample)
    : id(sourceSample.id)
    , name(sourceSample.name)
    , description(sourceSample.description)
    , material(sourceSample.material)
    , date(sourceSample.date)
{

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
    return material.getName();
}

Material::Category Sample::getMaterialCategory() const
{
    return material.getCategory();
}

QString Sample::getMaterialDescription() const
{
    return material.getDescription();
}

double Sample::getMaterialDensity() const
{
    return material.getDensity();
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

void Sample::setMaterial(const Material &newMaterial)
{
    material = newMaterial;
}

QJsonObject Sample::toJson() const
{
    QJsonObject sampleObj;
    sampleObj["id"] = getId();
    sampleObj["name"] = getName();
    sampleObj["description"] = getDescription();
    sampleObj["date"] = getDate().toString(Qt::ISODate);
    sampleObj["material"] = material.toJson();

    return sampleObj;
}

void Sample::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setName(json["name"].toString());
    setDescription(json["description"].toString());
    setDate(QDateTime::fromString(json["date"].toString(), Qt::ISODate));
    if(json.contains("material") && json["material"].isObject())
        material.fromJson(json["material"].toObject());
}
