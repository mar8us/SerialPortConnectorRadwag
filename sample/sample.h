#ifndef SAMPLE_H
#define SAMPLE_H

#include <QString>
#include <QDateTime>
#include "../material_tabels/material.h"

class Sample
{
public:
    Sample();
    Sample(const QString &number, const QString &name,
           const QString &materialName, const QString &materialCategory,
           const QString &materialDescription, double materialDensity,
           const QString &description, const QString &author);

    QString getNumber() const;
    QString getName() const;
    QString getDescription() const;
    QString getAuthor() const;
    QDateTime getDate() const;

    QString getMaterialName() const;
    QString getMaterialCategory() const;
    QString getMaterialDescription() const;
    double getMaterialDensity() const;

    void setNumber(const QString &newNumber);
    void setName(const QString &newName);
    void setDescription(const QString &newDescription);
    void setAuthor(const QString &newAuthor);
    void setDate(const QDateTime &newDate);

    void setMaterialName(const QString &newMaterialName);
    void setMaterialCategory(const QString &newMaterialCategory);
    void setMaterialDescription(const QString &newMaterialDescription);
    void setMaterialDensity(double newMaterialDensity);

    void setMaterial(const QString &materialName, const QString &materialCategory,
                     const QString &materialDescription, double materialDensity);

    void setMaterial(const Material &material);

private:
    QString number;        // Unikalny numer próbki
    QString name;
    QString description;
    QString author;
    QDateTime date;

    QString materialName;
    QString materialCategory;
    QString materialDescription;
    double materialDensity;
};

#endif // SAMPLE_H
