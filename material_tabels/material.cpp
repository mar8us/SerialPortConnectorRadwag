#include "material.h"

Material::Material()
{

}

Material::Material(const QString &name, const Material::Category category, const QString &description, double density)
    : name(name), category(category), description(description), density(density)
{

}

Material::Material(const Material &sourceMaterial)
    : name(sourceMaterial.name), category(sourceMaterial.category), description(sourceMaterial.description), density(sourceMaterial.density)
{

}

QString Material::getName() const
{
    return name;
}

Material::Category Material::getCategory() const
{
    return category;
}

QString Material::getDescription() const
{
    return description;
}

double Material::getDensity() const
{
    return density;
}

QJsonObject Material::toJson() const
{
    QJsonObject materialObj;
    materialObj["name"] = name;
    materialObj["category"] = static_cast<int>(category);
    materialObj["description"] = description;
    materialObj["density"] = density;

    return materialObj;
}

void Material::fromJson(const QJsonObject &json)
{
    if(json.contains("name"))
        name = json["name"].toString();

    if(json.contains("category"))
        category = static_cast<Category>(json["category"].toInt());

    if(json.contains("description"))
        description = json["description"].toString();

    if(json.contains("density"))
        density = json["density"].toDouble();
}
