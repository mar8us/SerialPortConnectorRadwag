#include "sample.h"
#include <qjsonobject.h>

Sample::Sample()
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const QString &name, const QString &description, const Material &material)
    : name(name)
    , description(description)
    , material(material)
{
    date = QDateTime::currentDateTime();
}

Sample::Sample(const Sample &sourceSample)
    : name(sourceSample.name)
    , description(sourceSample.description)
    , material(sourceSample.material)
    , date(sourceSample.date)
{

}

Sample::~Sample()
{

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

const Material &Sample::getMaterial() const
{
    return material;
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

void Sample::setName(const QString &newName) const
{
    name = newName;
}

void Sample::setDescription(const QString &newDescription) const
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
    sampleObj["name"] = getName();
    sampleObj["description"] = getDescription();
    sampleObj["date"] = getDate().toString(Qt::ISODate);
    sampleObj["material"] = material.toJson();

    return sampleObj;
}

void Sample::fromJson(const QJsonObject &json)
{
    setName(json["name"].toString());
    setDescription(json["description"].toString());
    setDate(QDateTime::fromString(json["date"].toString(), Qt::ISODate));
    if(json.contains("material") && json["material"].isObject())
        material.fromJson(json["material"].toObject());
}
