#ifndef SAMPLE_H
#define SAMPLE_H

#include <QString>
#include <QDateTime>
#include "../material_tabels/material.h"

class Sample
{
public:
    Sample();
    Sample(const QString &name, const QString &description, const Material &material);
    Sample(const Sample &sourceSample);
    ~Sample();

    QString getName() const;
    QString getDescription() const;
    QDateTime getDate() const;

    const Material &getMaterial() const;
    QString getMaterialName() const;
    Material::Category getMaterialCategory() const;
    QString getMaterialDescription() const;
    double getMaterialDensity() const;

    void setName(const QString &newName) const;
    void setDescription(const QString &newDescription) const;
    void setDate(const QDateTime &newDate);
    void setMaterial(const Material &material);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    mutable QString name;
    mutable QString description;
    QDateTime date;

    Material material;
};

#endif // SAMPLE_H
