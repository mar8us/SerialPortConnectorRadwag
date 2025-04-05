#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <qjsonobject.h>

class Material
{
public:
    enum class Category
    {
        Metal,
        Ceramic,
        Polymer,
        Composite,
        Glass,
        Rock,
        Other
    };

    Material();
    Material(const QString &name, const Material::Category category, const QString &description, double density);
    Material(const Material &sourceMaterial);
    QString getName() const;
    Category getCategory() const;
    QString getDescription() const;
    double getDensity() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
private:
    QString name;
    Category category;
    QString description;
    double density;
};
#endif // MATERIAL_H
