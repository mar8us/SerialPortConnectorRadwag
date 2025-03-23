#include "material.h"

Material::Material()
{

}

Material::Material(const QString &name, const QString &category, const QString &description, double density)
    : name(name), category(category), description(description), density(density)
{

}

QString Material::getName() const
{
    return name;
}

QString Material::getCategory() const
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
