#ifndef SAMPLE_H
#define SAMPLE_H

#include <QString>
#include <QDateTime>
#include "../material_tabels/material.h"

class Sample
{
public:
    Sample();
    Sample(const QString &id, const QString &name, const QString &description, const Material &material);
    Sample(const Sample &sourceSample);

    QString getId() const;
    QString getName() const;
    QString getDescription() const;
    QDateTime getDate() const;

    QString getMaterialName() const;
    Material::Category getMaterialCategory() const;
    QString getMaterialDescription() const;
    double getMaterialDensity() const;

    void setId(const QString &newId);
    void setName(const QString &newName);
    void setDescription(const QString &newDescription);
    void setDate(const QDateTime &newDate);
    void setMaterial(const Material &material);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString id;
    QString name;
    QString description;
    QDateTime date;

    Material material;
};

#endif // SAMPLE_H
