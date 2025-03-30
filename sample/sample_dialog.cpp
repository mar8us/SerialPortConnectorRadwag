#include "sample_dialog.h"
#include "ui_sample_dialog.h"
#include "../material_tabels/material_tabels_dialog.h"

SampleDialog::SampleDialog(QMap<QString, Sample> &samples, QMap<QString, Material> &materials, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SampleDialog),
    samples(samples),
    materials(materials),
    modified(false)
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
        updateSampleDetails(ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 1)->text());
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

QMap<QString, Sample> SampleDialog::getSamples() const
{
    return samples;
}

void SampleDialog::buttonAddSampleOnClicked()
{
    clearSampleDetails();
    ui->groupBoxSampleDetails->setEnabled(true);
    ui->editSampleId->setText(generateSampleId());
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

    editingSampleId = ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 1)->text();
    ui->groupBoxSampleDetails->setEnabled(true);
    modified = true;
    updateButtonsState();
}

void SampleDialog::buttonRemoveSampleOnClicked()
{
    if(ui->tableWidgetSamples->selectedItems().isEmpty() || ui->groupBoxSampleDetails->isEnabled())
        return;

    int currentRow = ui->tableWidgetSamples->currentRow();
    QString sampleId = ui->tableWidgetSamples->item(currentRow, 1)->text();
    QString sampleName = ui->tableWidgetSamples->item(currentRow, 0)->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Potwierdzenie usunięcia", QString("Czy na pewno chcesz usunąć próbkę '%1' (%2)?").arg(sampleName).arg(sampleId), QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        samples.remove(sampleId);
        ui->tableWidgetSamples->removeRow(currentRow);
        clearSampleDetails();

        if(ui->tableWidgetSamples->rowCount() == 0)
            ui->groupBoxSampleDetails->setEnabled(false);

        emit samplesChanged();
    }

    if(!ui->tableWidgetSamples->selectedItems().isEmpty())
        updateSampleDetails(ui->tableWidgetSamples->item(ui->tableWidgetSamples->currentRow(), 1)->text());
}

void SampleDialog::buttonSaveSampleOnClicked()
{
    if(!validateSampleDetails())
        return;

    QString id = ui->editSampleId->text().trimmed();
    QString name = ui->editSampleName->text().trimmed();
    QString materialName = ui->comboBoxMaterial->currentText();
    double materialDensity = ui->editMaterialDensity->text().toDouble();
    QString description = ui->editSampleDescription->toPlainText();

    auto material = materials.value(materialName);
    Sample sample(id, name, material.getName(), material.getCategory(), material.getDescription(), material.getDensity(), description);

    if(editingSampleId.isEmpty())
    {
        int row = ui->tableWidgetSamples->rowCount();
        ui->tableWidgetSamples->insertRow(row);
        ui->tableWidgetSamples->setItem(row, 0, new QTableWidgetItem(sample.getName()));
        ui->tableWidgetSamples->setItem(row, 1, new QTableWidgetItem(sample.getId()));
    }
    else
    {
        samples.remove(editingSampleId);
        for(int row = 0; row < ui->tableWidgetSamples->rowCount(); row++)
        {
            if(ui->tableWidgetSamples->item(row, 1)->text() != editingSampleId)
                continue;
            ui->tableWidgetSamples->setItem(row, 0, new QTableWidgetItem(sample.getName()));
            ui->tableWidgetSamples->setItem(row, 1, new QTableWidgetItem(sample.getId()));
            break;
        }
    }

    samples[sample.getId()] = sample;
    editingSampleId.clear();
    modified = false;

    ui->groupBoxSampleDetails->setEnabled(false);

    for(int row = 0; row < ui->tableWidgetSamples->rowCount(); row++)
    {
        if(ui->tableWidgetSamples->item(row, 1)->text() != sample.getId())
            continue;
        ui->tableWidgetSamples->selectRow(row);
        break;
    }

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
    int row = 0;
    for(auto sampleIt = samples.begin(); sampleIt != samples.end(); sampleIt++)
    {
        ui->tableWidgetSamples->insertRow(row);
        ui->tableWidgetSamples->setItem(row, 0, new QTableWidgetItem(sampleIt->getName()));
        ui->tableWidgetSamples->setItem(row, 1, new QTableWidgetItem(sampleIt->getId()));
        row++;
    }

    if(ui->tableWidgetSamples->rowCount() > 0)
        ui->tableWidgetSamples->selectRow(0);
}

bool SampleDialog::validateSampleDetails()
{
    QString sampleId = ui->editSampleId->text().trimmed();
    if(sampleId.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak ID"), tr("Pole ID próbki nie może być puste."), QMessageBox::Ok);
        ui->editSampleId->setFocus();
        return false;
    }

    if(editingSampleId != sampleId && !isUniqueSampleId(sampleId))
    {
        QMessageBox::warning(this, "Błąd zapisu", "Próbka o tym ID już istnieje.");
        return false;
    }

    QString sampleName = ui->editSampleName->text().trimmed();
    if(sampleName.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak nazwy"), tr("Pole nazwy próbki nie może być puste."), QMessageBox::Ok);
        ui->editSampleName->setFocus();
        return false;
    }

    return true;
}

void SampleDialog::updateSampleDetails(const QString &sampleId)
{
    clearSampleDetails();
    if(!samples.contains(sampleId))
        return;

    const Sample &sample = samples[sampleId];

    ui->editSampleId->setText(sample.getId());
    ui->editSampleName->setText(sample.getName());

    int materialIndex = ui->comboBoxMaterial->findText(sample.getMaterialName());
    if(materialIndex >= 0)
        ui->comboBoxMaterial->setCurrentIndex(materialIndex);

    ui->editMaterialDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 4));
    ui->editSampleDescription->setPlainText(sample.getDescription());
}

void SampleDialog::clearSampleDetails()
{
    ui->editSampleId->clear();
    ui->editSampleName->clear();
    ui->comboBoxMaterial->setCurrentIndex(-1);
    ui->editMaterialDensity->clear();
    ui->editSampleDescription->clear();
    editingSampleId.clear();
    modified = false;
}

QString SampleDialog::generateSampleId() const
{
    int counter = 1;
    QString newSampleId;
    do
    {
        newSampleId = "PRB-" + QString("%1").arg(counter, 3, 10, QChar('0'));
        counter++;
    } while(samples.contains(newSampleId));

    return newSampleId;
}

bool SampleDialog::isUniqueSampleId(const QString &id) const
{
    return !samples.contains(id);
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
    connect(ui->editSampleId, &QLineEdit::textChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && true;
    });

    connect(ui->editSampleName, &QLineEdit::textChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && true;
    });

    connect(ui->comboBoxMaterial, &QComboBox::currentTextChanged, [this]() {
        modified = ui->groupBoxSampleDetails->isEnabled() && true;
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
        updateSampleDetails(ui->tableWidgetSamples->item(currentRow, 1)->text());
        updateButtonsState();
    }
}

void SampleDialog::comboBoxMaterialOnCurrentIndexChanged(const QString &materialName)
{
    if(materials.contains(materialName))
    {
        double density = materials[materialName].getDensity();
        ui->editMaterialDensity->setText(QString::number(density, 'f', 3));
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
