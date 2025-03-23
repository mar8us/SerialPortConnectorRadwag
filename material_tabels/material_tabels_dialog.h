#ifndef MATERIAL_TABELS_DIALOG_H
#define MATERIAL_TABELS_DIALOG_H

#include "material.h"
#include <QDialog>
#include <QMap>
#include <QListWidgetItem>
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
class MaterialTabelsDialog;
}

class MaterialTablesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialTablesDialog(QMap<QString, Material> &materials, QWidget *parent = nullptr);
    ~MaterialTablesDialog();
    QMap<QString, Material> getMaterials() const;

private slots:
    void buttonAddMaterialOnClicked();
    void buttonEditMaterialOnClicked();
    void buttonRemoveMaterialOnClicked();
    void buttonSaveMaterialOnClicked();
    void buttonCancelEditMaterialOnClicked();
    void buttonCloseOnClicked();
    void listWidgetMaterialsOnCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MaterialTabelsDialog *ui;
    QMap<QString, Material> &materials;
    bool modified;
    QString editingMaterialName;

    void updateMaterialList();
    bool validateMaterialDetails();
    void updateMaterialDetails(const QString &materialName);
    void clearMaterialDetails();
    QString getUniqueMaterialName();
    bool isUniqueMaterialName(const QString &name) const;
    void loadMaterials();
    void saveMaterials();
    void connectSignalsAndSlots();
    void connectSignalsForModification();
    void updateButtonsState();
};

#endif // MATERIAL_TABELS_DIALOG_H
