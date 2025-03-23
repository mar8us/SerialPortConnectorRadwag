#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>

class Material
{
public:
    Material();
    Material(const QString &name, const QString &category, const QString &description, double density);
    QString getName() const;
    QString getCategory() const;
    QString getDescription() const;
    double getDensity() const;

private:
    QString name;
    QString category;
    QString description;
    double density;
};
#endif // MATERIAL_H
