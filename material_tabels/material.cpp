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
