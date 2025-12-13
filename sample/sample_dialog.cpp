#include "sample_dialog.h"
#include "ui_sample_dialog.h"
#include "../material_tabels/material_tabels_dialog.h"

SampleDialog::SampleDialog(SampleManager* sampleManager, QMap<QString, Material> &materials, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SampleDialog)
    , sampleManager(sampleManager)
    , materials(materials)
    , modified(false)
{
    ui->setupUi(this);
    ui->groupBoxSampleDetails->setEnabled(false);
    ui->tableWidgetSamples->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidgetSamples->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    updateSampleList();
    fillMaterialCombo();
    connectSignalsAndSlots();
    connectSignalsForModification();
    if(ui->tableWidgetSamples->selectedItems().count() > 0)
        updateSampleDetails(ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 0)->text());
}

void SampleDialog::fillMaterialCombo()
{
    ui->comboBoxMaterial->clear();
    for(const auto &material : materials)
        ui->comboBoxMaterial->addItem(material.getName());
    ui->comboBoxMaterial->setCurrentIndex(-1);
}

SampleDialog::~SampleDialog()
{
    delete ui;
}

void SampleDialog::buttonAddSampleOnClicked()
{
    clearSampleDetails();
    ui->groupBoxSampleDetails->setEnabled(true);

    ui->comboBoxMaterial->setEnabled(true);

    modified = true;
    updateButtonsState();
}

void SampleDialog::buttonEditSampleOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        return;
    }
    if(ui->tableWidgetSamples->selectedItems().isEmpty())
        return;

    editingSampleName = ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 0)->text();
    ui->groupBoxSampleDetails->setEnabled(true);

    ui->comboBoxMaterial->setEnabled(false);

    modified = true;
    updateButtonsState();
}

void SampleDialog::buttonRemoveSampleOnClicked()
{
    if(ui->tableWidgetSamples->selectedItems().isEmpty() || ui->groupBoxSampleDetails->isEnabled())
        return;

    int currentRow = ui->tableWidgetSamples->currentRow();
    QString sampleName = ui->tableWidgetSamples->item(currentRow, 0)->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Potwierdzenie usunięcia",
                                  QString("Czy na pewno chcesz usunąć wybraną serie '%1'? Spowoduje to usunięcie wszyskich powiązany pomiarów.").arg(sampleName),
                                  QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        auto sample = sampleManager->getSample(sampleName);
        if(sample)
        {
            sampleManager->removeSample(sampleName);

            ui->tableWidgetSamples->removeRow(currentRow);
            clearSampleDetails();

            if(ui->tableWidgetSamples->rowCount() == 0)
                ui->groupBoxSampleDetails->setEnabled(false);

            emit samplesChanged();
        }
    }

    if(!ui->tableWidgetSamples->selectedItems().isEmpty())
        updateSampleDetails(ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 0)->text());
}

void SampleDialog::buttonSaveSampleOnClicked()
{
    if(!validateSampleDetails())
        return;

    QString name = ui->editSampleName->text().trimmed();
    QString description = ui->editSampleDescription->toPlainText();

    std::shared_ptr<const Sample> newSample;

    int row = -1;
    if(editingSampleName.isEmpty())
    {
        newSample = std::make_shared<const Sample>(name, description, materials.value(ui->comboBoxMaterial->currentText()));

        sampleManager->addSample(newSample);

        row = ui->tableWidgetSamples->rowCount();
        ui->tableWidgetSamples->insertRow(row);
        setSampleToRow(row, newSample);
    }
    else
    {
        auto editedSample = sampleManager->getSample(editingSampleName);
        if(!editedSample)
        {
            QMessageBox::warning(this, "Błąd", "Nie znaleziono serii.");
            return;
        }
        QString oldName = editedSample->getName();
        editedSample->setName(name);
        editedSample->setDescription(description);
        sampleManager->updateSample(oldName, editedSample);
    }
    modified = false;
    editingSampleName.clear();
    updateSampleList();
    if(row != -1)
        ui->tableWidgetSamples->selectRow(row);
    ui->groupBoxSampleDetails->setEnabled(false);
    updateButtonsState();
    emit samplesChanged();
}

void SampleDialog::buttonCancelEditSampleOnClicked()
{
    clearSampleDetails();
    ui->groupBoxSampleDetails->setEnabled(false);
    modified = false;
    updateButtonsState();
}

void SampleDialog::updateSampleList()
{
    ui->tableWidgetSamples->setRowCount(0);

    const auto& samples = sampleManager->getSamples();
    int row = 0;
    for(auto sampleIt = samples.begin(); sampleIt != samples.end(); ++sampleIt)
    {
        ui->tableWidgetSamples->insertRow(row);
        setSampleToRow(row, sampleIt.value());
        row++;
    }
}

std::shared_ptr<const Sample> SampleDialog::getSampleFromRow(int row) const
{
    if(row < 0 || row >= ui->tableWidgetSamples->rowCount())
        return nullptr;

    QTableWidgetItem* item = ui->tableWidgetSamples->item(row, 0);
    if(!item)
        return nullptr;

    QString sampleName = item->text();
    return sampleManager->getSample(sampleName);
}

void SampleDialog::setSampleToRow(int row, std::shared_ptr<const Sample> sample)
{
    if(row < 0 || row >= ui->tableWidgetSamples->rowCount())
        return;

    QTableWidgetItem* item = ui->tableWidgetSamples->item(row, 0);
    if(!item)
    {
        item = new QTableWidgetItem();
        ui->tableWidgetSamples->setItem(row, 0, item);
    }

    item->setText(sample->getName());
}

bool SampleDialog::validateSampleDetails()
{
    QString sampleName = ui->editSampleName->text().trimmed();
    if(sampleName.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak nazwy"), tr("Pole nazwy próbki nie może być puste."), QMessageBox::Ok);
        ui->editSampleName->setFocus();
        return false;
    }

    if(editingSampleName != sampleName && sampleManager->sampleExists(sampleName))
    {
        QMessageBox::warning(this, "Błąd zapisu", "Próbka o tej nazwie już istnieje.");
        ui->editSampleName->setFocus();
        return false;
    }

    if(editingSampleName.isEmpty() && ui->comboBoxMaterial->currentText().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak materiału"), tr("Musisz wybrać materiał."), QMessageBox::Ok);
        ui->comboBoxMaterial->setFocus();
        return false;
    }

    return true;
}

void SampleDialog::updateSampleDetails(const QString &sampleName)
{
    clearSampleDetails();

    auto sample = sampleManager->getSample(sampleName);
    if(!sample)
        return;

    ui->editSampleName->setText(sample->getName());

    int materialIndex = ui->comboBoxMaterial->findText(sample->getMaterialName());
    if(materialIndex >= 0)
        ui->comboBoxMaterial->setCurrentIndex(materialIndex);

    ui->editMaterialDensity->setText(QString::number(sample->getMaterialDensity(), 'f', 5) + " g/cm³");
    ui->editSampleDescription->setPlainText(sample->getDescription());
}

void SampleDialog::clearSampleDetails()
{
    ui->editSampleName->clear();
    ui->comboBoxMaterial->setCurrentIndex(-1);
    ui->comboBoxMaterial->setEnabled(true);
    ui->editMaterialDensity->clear();
    ui->editSampleDescription->clear();
    editingSampleName.clear();
    modified = false;
}

void SampleDialog::connectSignalsAndSlots()
{
    connect(ui->buttonAddSample, &QPushButton::clicked, this, &SampleDialog::buttonAddSampleOnClicked);
    connect(ui->buttonEditSample, &QPushButton::clicked, this, &SampleDialog::buttonEditSampleOnClicked);
    connect(ui->buttonRemoveSample, &QPushButton::clicked, this, &SampleDialog::buttonRemoveSampleOnClicked);

    connect(ui->buttonSaveSample, &QPushButton::clicked, this, &SampleDialog::buttonSaveSampleOnClicked);
    connect(ui->buttonCancelEditSample, &QPushButton::clicked, this, &SampleDialog::buttonCancelEditSampleOnClicked);

    connect(ui->buttonClose, &QPushButton::clicked, this, &SampleDialog::buttonCloseOnClicked);

    connect(ui->tableWidgetSamples, &QTableWidget::itemSelectionChanged, this, &SampleDialog::tableWidgetSamplesOnSelectionChanged);

    connect(ui->comboBoxMaterial, &QComboBox::currentTextChanged, this, &SampleDialog::comboBoxMaterialOnCurrentIndexChanged);

    connect(ui->buttonTableMatrials, &QPushButton::clicked, this, &SampleDialog::buttonTableMatrialsOnClicked);
}

void SampleDialog::connectSignalsForModification()
{
    connect(ui->editSampleName, &QLineEdit::textChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && true;
    });

    connect(ui->comboBoxMaterial, &QComboBox::currentTextChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && ui->comboBoxMaterial->isEnabled() && true;
    });

    connect(ui->editSampleDescription, &QPlainTextEdit::textChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && true;
    });
}

void SampleDialog::updateButtonsState()
{
    ui->groupBoxSampleList->setEnabled(!modified);
}

void SampleDialog::buttonCloseOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        return;
    }
    accept();
}

void SampleDialog::tableWidgetSamplesOnSelectionChanged()
{
    if(ui->tableWidgetSamples->selectedItems().isEmpty())
        return;

    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycję"), QMessageBox::Ok);
        return;
    }

    int currentRow = ui->tableWidgetSamples->currentRow();
    if(currentRow >= 0)
    {
        ui->groupBoxSampleDetails->setEnabled(false);
        updateSampleDetails(ui->tableWidgetSamples->item(currentRow, 0)->text());
        updateButtonsState();
    }
}

void SampleDialog::comboBoxMaterialOnCurrentIndexChanged(const QString &materialName)
{
    if(materials.contains(materialName))
    {
        double density = materials[materialName].getDensity();
        ui->editMaterialDensity->setText(QString::number(density, 'f', 5) +  + " g/cm³");
    }
    else
        ui->editMaterialDensity->clear();
}

void SampleDialog::buttonTableMatrialsOnClicked()
{
    auto dialog = new MaterialTablesDialog(materials, this);
    dialog->exec();
    fillMaterialCombo();
    emit materialsChanged(materials);
}
