#ifndef SAMPLE_H
#define SAMPLE_H

#include <QString>
#include <QDateTime>
#include "../material_tabels/material.h"

class Sample
{
public:
    Sample();
    Sample(const QString &id, const QString &name,
           const QString &materialName, const QString &materialCategory,
           const QString &materialDescription, double materialDensity,
           const QString &description);

    QString getId() const;
    QString getName() const;
    QString getDescription() const;
    QDateTime getDate() const;

    QString getMaterialName() const;
    QString getMaterialCategory() const;
    QString getMaterialDescription() const;
    double getMaterialDensity() const;

    void setId(const QString &newId);
    void setName(const QString &newName);
    void setDescription(const QString &newDescription);
    void setDate(const QDateTime &newDate);

    void setMaterialName(const QString &newMaterialName);
    void setMaterialCategory(const QString &newMaterialCategory);
    void setMaterialDescription(const QString &newMaterialDescription);
    void setMaterialDensity(double newMaterialDensity);

    void setMaterial(const QString &materialName, const QString &materialCategory,
                     const QString &materialDescription, double materialDensity);

    void setMaterial(const Material &material);

private:
    QString id;        // Unikalne id próbki
    QString name;
    QString description;
    QDateTime date;

    QString materialName;
    QString materialCategory;
    QString materialDescription;
    double materialDensity;
};

#endif // SAMPLE_H
