#include "material_tabels_dialog.h"
#include "ui_material_tabels_dialog.h"

MaterialTablesDialog::MaterialTablesDialog(QMap<QString, Material> &materials, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MaterialTabelsDialog),
    modified(false),
    materials(materials)
{
    ui->setupUi(this);
    ui->groupBoxMaterialDetails->setEnabled(false);
    fillComboMaterialCategory();

    connectSignalsAndSlots();
    connectSignalsForModification();
    updateMaterialList();
    //loadMaterials();
}

MaterialTablesDialog::~MaterialTablesDialog()
{
    delete ui;
}

QMap<QString, Material> MaterialTablesDialog::getMaterials() const
{
    return materials;
}

void MaterialTablesDialog::buttonAddMaterialOnClicked()
{
    clearMaterialDetails();
    ui->groupBoxMaterialDetails->setEnabled(true);
    ui->editMaterialName->setText(getUniqueMaterialName());
    modified = true;
    updateButtonsState();
}

void MaterialTablesDialog::buttonEditMaterialOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        return;
    }
    if(!ui->listWidgetMaterials->currentItem())
        return;

    editingMaterialName = ui->listWidgetMaterials->currentItem()->text();
    ui->groupBoxMaterialDetails->setEnabled(true);
    modified = true;
    updateButtonsState();
}

void MaterialTablesDialog::buttonRemoveMaterialOnClicked()
{
    QListWidgetItem* currentItem = ui->listWidgetMaterials->currentItem();
    if(!currentItem || ui->groupBoxMaterialDetails->isEnabled())
        return;

    QString materialName = currentItem->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Potwierdzenie usunięcia", QString("Czy na pewno chcesz usunąć materiał '%1'?").arg(materialName), QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        materials.remove(materialName);
        delete ui->listWidgetMaterials->takeItem(ui->listWidgetMaterials->row(currentItem));
        clearMaterialDetails();

        if(ui->listWidgetMaterials->count() == 0)
            ui->groupBoxMaterialDetails->setEnabled(false);
    }

    if(ui->listWidgetMaterials->selectedItems().count() > 0)
        updateMaterialDetails(ui->listWidgetMaterials->selectedItems().first()->text());
}

void MaterialTablesDialog::buttonSaveMaterialOnClicked()
{
    if(!validateMaterialDetails())
        return;

    QString name = ui->editMaterialName->text().trimmed();
    Material::Category category = static_cast<Material::Category>(ui->comboMaterialCategory->currentData().toInt());
    QString description = ui->editMaterialDescription->toPlainText();
    double density = ui->spinTheoreticalDensity->value();

    Material material(name, category, description, density);

    if(editingMaterialName.isEmpty())
        ui->listWidgetMaterials->addItem(material.getName());
    else
    {
        materials.remove(editingMaterialName);
        for(int i = 0; i < ui->listWidgetMaterials->count(); i++)
        {
            if(ui->listWidgetMaterials->item(i)->text() != editingMaterialName)
                continue;
            ui->listWidgetMaterials->item(i)->setText(material.getName());
            break;
        }
    }

    materials[material.getName()] = material;
    editingMaterialName.clear();
    modified = false;

    ui->groupBoxMaterialDetails->setEnabled(false);
    ui->listWidgetMaterials->setCurrentItem(*ui->listWidgetMaterials->findItems(material.getName(), Qt::MatchFlag::MatchExactly).begin());
    updateButtonsState();
}

void MaterialTablesDialog::buttonCancelEditMaterialOnClicked()
{
    clearMaterialDetails();
    ui->groupBoxMaterialDetails->setEnabled(false);
    modified = false;
    updateButtonsState();
}

void MaterialTablesDialog::updateMaterialList()
{
    for(auto materialIt = materials.begin(); materialIt != materials.end(); materialIt++)
        ui->listWidgetMaterials->addItem(materialIt->getName());
    if(ui->listWidgetMaterials->count() > 0)
        ui->listWidgetMaterials->setCurrentRow(0);
}

bool MaterialTablesDialog::validateMaterialDetails()
{
    QString materialName = ui->editMaterialName->text().trimmed();
    if(materialName.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak nazwy"), tr("Pole nazwy materiału nie może być puste."), QMessageBox::Ok);
        ui->editMaterialName->setFocus();
        return false;
    }

    if(editingMaterialName != materialName && !isUniqueMaterialName(materialName))
    {
        QMessageBox::warning(this, "Błąd zapisu", "Materiał o tej nazwie już istnieje.");
        return false;
    }

    double density = ui->spinTheoreticalDensity->value();
    if(density <= 0)
    {
        QMessageBox::warning(this, tr("Błędna gęstość"), tr("Gęstość materiału musi być wartością dodatnią."), QMessageBox::Ok);
        ui->spinTheoreticalDensity->setFocus();
        return false;
    }

    return true;
}

void MaterialTablesDialog::updateMaterialDetails(const QString &materialName)
{
    clearMaterialDetails();
    if(!materials.contains(materialName))
        return;

    const Material &material = materials[materialName];

    ui->editMaterialName->setText(material.getName());

    int categoryIndex = ui->comboMaterialCategory->findData(static_cast<int>(material.getCategory()));
    if (categoryIndex >= 0)
        ui->comboMaterialCategory->setCurrentIndex(categoryIndex);

    ui->editMaterialDescription->setPlainText(material.getDescription());
    ui->spinTheoreticalDensity->setValue(material.getDensity());
}

void MaterialTablesDialog::clearMaterialDetails()
{
    ui->editMaterialName->clear();
    ui->comboMaterialCategory->setCurrentIndex(0);
    ui->editMaterialDescription->clear();
    ui->spinTheoreticalDensity->setValue(0.0);
    editingMaterialName.clear();
    modified = false;
}

QString MaterialTablesDialog::getUniqueMaterialName()
{
    QString newMaterialName = "Nowy materiał";
    int counter = 1;
    while(materials.contains(newMaterialName))
    {
        newMaterialName = QString("Nowy materiał %1").arg(counter);
        counter++;
    }
    return newMaterialName;
}

bool MaterialTablesDialog::isUniqueMaterialName(const QString &name) const
{
    return materials.find(name) == materials.end();
}

void MaterialTablesDialog::connectSignalsAndSlots()
{
    connect(ui->buttonAddMaterial, &QPushButton::clicked, this, &MaterialTablesDialog::buttonAddMaterialOnClicked);
    connect(ui->buttonEditMaterial, &QPushButton::clicked, this, &MaterialTablesDialog::buttonEditMaterialOnClicked);
    connect(ui->buttonRemoveMaterial, &QPushButton::clicked, this, &MaterialTablesDialog::buttonRemoveMaterialOnClicked);

    connect(ui->buttonSaveMaterial, &QPushButton::clicked, this, &MaterialTablesDialog::buttonSaveMaterialOnClicked);
    connect(ui->buttonCancelEditMaterial, &QPushButton::clicked, this, &MaterialTablesDialog::buttonCancelEditMaterialOnClicked);

    connect(ui->buttonClose, &QPushButton::clicked, this, &MaterialTablesDialog::buttonCloseOnClicked);

    connect(ui->listWidgetMaterials, &QListWidget::currentItemChanged, this, &MaterialTablesDialog::listWidgetMaterialsOnCurrentItemChanged);
}

void MaterialTablesDialog::connectSignalsForModification()
{
    connect(ui->editMaterialName, &QLineEdit::textChanged, [this]() {
        modified = ui->groupBoxMaterialDetails->isEnabled() && true;
    });

    connect(ui->comboMaterialCategory, &QComboBox::currentTextChanged, [this]() {
        modified = ui->groupBoxMaterialDetails->isEnabled() && true;
    });

    connect(ui->editMaterialDescription, &QPlainTextEdit::textChanged, [this]() {
        modified = ui->groupBoxMaterialDetails->isEnabled() && true;
    });

    connect(ui->spinTheoreticalDensity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        modified = ui->groupBoxMaterialDetails->isEnabled() && true;
    });
}

void MaterialTablesDialog::updateButtonsState()
{
    ui->groupBoxMaterialList->setEnabled(!modified);
}

void MaterialTablesDialog::buttonCloseOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        return;
    }
    //saveMaterials();
    accept();
}

void MaterialTablesDialog::listWidgetMaterialsOnCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(!current)
        return;

    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        ui->listWidgetMaterials->setCurrentItem(previous);
        return;
    }

    ui->groupBoxMaterialDetails->setEnabled(false);
    updateMaterialDetails(current->text());
    updateButtonsState();
}

void MaterialTablesDialog::loadMaterials()
{
    QSettings settings;
    QString filePath = settings.value("MaterialTablesPath", QCoreApplication::applicationDirPath() + "/materials.json").toString();

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    file.close();

    if(doc.isArray())
    {
        QJsonArray materialsArray = doc.array();

        for(int i = 0; i < materialsArray.size(); ++i)
        {
            QJsonObject materialObj = materialsArray[i].toObject();

            Material material(
                materialObj["name"].toString(),
                static_cast<Material::Category>(materialObj["category"].toInt()),
                materialObj["description"].toString(),
                materialObj["density"].toDouble()
                );

            materials[material.getName()] = material;
            ui->listWidgetMaterials->addItem(material.getName());
        }
    }

    if(ui->listWidgetMaterials->count() > 0)
        ui->listWidgetMaterials->setCurrentRow(0);
}

void MaterialTablesDialog::saveMaterials()
{
    QJsonArray materialsArray;

    for(auto it = materials.begin(); it != materials.end(); ++it)
    {
        Material material = it.value();
        QJsonObject materialObj;
        materialObj["name"] = material.getName();
        materialObj["category"] = static_cast<int>(material.getCategory());
        materialObj["description"] = material.getDescription();
        materialObj["density"] = material.getDensity();

        materialsArray.append(materialObj);
    }

    QJsonDocument doc(materialsArray);

    QSettings settings;
    QString filePath = settings.value("MaterialTablesPath", QCoreApplication::applicationDirPath() + "/materials.json").toString();

    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
    }
    else
        QMessageBox::warning(this, "Błąd zapisu", "Nie można zapisać danych materiałów do pliku.");
}

void MaterialTablesDialog::fillComboMaterialCategory()
{
    ui->comboMaterialCategory->clear();
    ui->comboMaterialCategory->addItem("Metal", static_cast<int>(Material::Category::Metal));
    ui->comboMaterialCategory->addItem("Ceramika", static_cast<int>(Material::Category::Ceramic));
    ui->comboMaterialCategory->addItem("Polimer", static_cast<int>(Material::Category::Polymer));
    ui->comboMaterialCategory->addItem("Kompozyt", static_cast<int>(Material::Category::Composite));
    ui->comboMaterialCategory->addItem("Szkło", static_cast<int>(Material::Category::Glass));
    ui->comboMaterialCategory->addItem("Skała", static_cast<int>(Material::Category::Rock));
    ui->comboMaterialCategory->addItem("Inny", static_cast<int>(Material::Category::Other));
}
