// fluidtablesdialog.cpp
#include "fluid_tables_form.h"
#include "ui_fluid_tables_form.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

FluidTablesDialog::FluidTablesDialog(QMap<QString, Fluid> &fluids, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FluidTablesDialog),
    modified(false),
    fluids(fluids)
{
    ui->setupUi(this);
    ui->tableWidgetDensity->setColumnCount(2);
    ui->tableWidgetDensity->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetDensity->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->groupBoxFluidDetails->setEnabled(false);

    connectSignalsAndSlots();
    connectSignalsForModification();
    updateFluidList();
    //loadFluids();
}

FluidTablesDialog::~FluidTablesDialog()
{
    delete ui;
}

QMap<QString, Fluid> FluidTablesDialog::getFluids() const
{
    return fluids;
}

void FluidTablesDialog::buttonAddFluidOnClicked()
{
    clearFluidDetails();
    ui->groupBoxFluidDetails->setEnabled(true);
    ui->editFluidName->setText(getUniqueFluidName());
    modified = true;
    updateButtonsState();
}

void FluidTablesDialog::buttonEditFluidOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycje"), QMessageBox::Ok);
        return;
    }
    editingFluidName = ui->listWidgetFluids->currentItem()->text();
    ui->groupBoxFluidDetails->setEnabled(true);
    modified = true;
    updateButtonsState();
}

void FluidTablesDialog::buttonRemoveFluidOnClicked()
{
    QListWidgetItem* currentItem = ui->listWidgetFluids->currentItem();
    if(!currentItem || ui->groupBoxFluidDetails->isEnabled())
        return;

    QString fluidName = currentItem->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Potwierdzenie usunięcia", QString("Czy na pewno chcesz usunąć ciecz '%1'?").arg(fluidName), QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        fluids.remove(fluidName);
        delete ui->listWidgetFluids->takeItem(ui->listWidgetFluids->row(currentItem));
        clearFluidDetails();

        if(ui->listWidgetFluids->count() == 0)
            ui->groupBoxFluidDetails->setEnabled(false);
    }
    updateFluidDetails(ui->listWidgetFluids->selectedItems().first()->text());
}

void FluidTablesDialog::buttonSaveEditFluidOnClicked()
{
    if(!validateFluidDetails())
        return;

    Fluid fluid(ui->editFluidName->text(), ui->editFluidDescription->toPlainText(), getDensityPointsFromTable());

    if(editingFluidName.isEmpty())
        ui->listWidgetFluids->addItem(fluid.getName());
    else
    {
        fluids.remove(editingFluidName);
        for(int i = 0; i < ui->listWidgetFluids->count(); i++)
        {
            if(ui->listWidgetFluids->item(i)->text() != editingFluidName)
                continue;
            ui->listWidgetFluids->item(i)->setText(fluid.getName());
            break;
        }
    }

    fluids[fluid.getName()] = fluid;
    editingFluidName.clear();
    modified = false;

    ui->groupBoxFluidDetails->setEnabled(false);
    ui->listWidgetFluids->setCurrentItem(*ui->listWidgetFluids->findItems(fluid.getName(), Qt::MatchFlag::MatchExactly).begin());
    updateButtonsState();
}

void FluidTablesDialog::buttonCancelEditFluidOnClicked()
{
    clearFluidDetails();
    ui->groupBoxFluidDetails->setEnabled(false);
    updateButtonsState();
}

void FluidTablesDialog::buttonAddRowOnClicked()
{
    int row = ui->tableWidgetDensity->rowCount();
    ui->tableWidgetDensity->insertRow(row);

    QTableWidgetItem* tempItem = new QTableWidgetItem("0.0");
    QTableWidgetItem* densityItem = new QTableWidgetItem("0.0");

    ui->tableWidgetDensity->setItem(row, 0, tempItem);
    ui->tableWidgetDensity->setItem(row, 1, densityItem);
}

void FluidTablesDialog::buttonRemoveRowOnClicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->tableWidgetDensity->selectedItems();
    if(selectedItems.isEmpty())
        return;

    QVector<int> rowsToRemove;
    rowsToRemove.reserve(selectedItems.size());
    for(QTableWidgetItem* item : selectedItems)
        rowsToRemove.push_back(item->row());
    std::sort(rowsToRemove.rbegin(), rowsToRemove.rend());

    for(int row : rowsToRemove)
        ui->tableWidgetDensity->removeRow(row);
}

void FluidTablesDialog::buttonImportCsvOnClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Importuj dane z CSV", "", "Pliki CSV (*.csv);;Wszystkie pliki (*)");
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Błąd importu", "Nie można otworzyć pliku.");
        return;
    }
    ui->tableWidgetDensity->setRowCount(0);

    QTextStream in(&file);

    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if(fields.size() < 2)
            continue;
        bool okTemp = false, okDensity = false;
        double temperature = fields[0].toDouble(&okTemp);
        double density = fields[1].toDouble(&okDensity);

        if(okTemp && okDensity)
        {
            int row = ui->tableWidgetDensity->rowCount();
            ui->tableWidgetDensity->insertRow(row);

            ui->tableWidgetDensity->setItem(row, 0, new QTableWidgetItem(QString::number(temperature, 'f', 2)));
            ui->tableWidgetDensity->setItem(row, 1, new QTableWidgetItem(QString::number(density, 'f', 5)));
        }
    }

    file.close();
}

bool FluidTablesDialog::validateFluidDetails()
{
    QString fluidName = ui->editFluidName->text().trimmed();
    if(fluidName.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak nazwy"), tr("Pole nazwy cieczy nie może być puste."), QMessageBox::Ok);

        ui->editFluidName->setFocus();
        return false;
    }

    if(editingFluidName != fluidName && !isUniqueFluidName(fluidName))
    {
        QMessageBox::warning(this, "Błąd zapisu", "Ciecz o tej nazwie już istnieje.");
        return false;
    }

    if(ui->tableWidgetDensity->rowCount() == 0)
    {
        QMessageBox::warning(this, tr("Brak danych"), tr("Tabela gęstości nie może być pusta. Dodaj przynajmniej jeden wiersz danych."), QMessageBox::Ok);
        return false;
    }

    for(int row = 0; row < ui->tableWidgetDensity->rowCount(); row++)
    {
        QTableWidgetItem* tempItem = ui->tableWidgetDensity->item(row, 0);
        QTableWidgetItem* densityItem = ui->tableWidgetDensity->item(row, 1);

        if(!tempItem || tempItem->text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, tr("Brak danych"), tr("Brak wartości temperatury w wierszu %1.").arg(row + 1), QMessageBox::Ok);
            ui->tableWidgetDensity->setCurrentCell(row, 0);
            return false;
        }

        if (!densityItem || densityItem->text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, tr("Brak danych"), tr("Brak wartości gęstości w wierszu %1.").arg(row + 1), QMessageBox::Ok);
            ui->tableWidgetDensity->setCurrentCell(row, 1);
            return false;
        }

        bool tempOk, densityOk;
        double temp = tempItem->text().toDouble(&tempOk);
        double density = densityItem->text().toDouble(&densityOk);

        if(!tempOk)
        {
            QMessageBox::warning(this, tr("Błędny format"), tr("Nieprawidłowy format temperatury w wierszu %1.").arg(row + 1), QMessageBox::Ok);
            ui->tableWidgetDensity->setCurrentCell(row, 0);
            return false;
        }

        if(!densityOk || density <= 0)
        {
            QMessageBox::warning(this, tr("Błędny format"), tr("Nieprawidłowy format gęstości w wierszu %1. Wartość musi być liczbą dodatnią.").arg(row + 1), QMessageBox::Ok);
            ui->tableWidgetDensity->setCurrentCell(row, 1);
            return false;
        }
    }

    return true;
}

void FluidTablesDialog::updateFluidDetails(const QString &fluidName)
{
    clearFluidDetails();
    if(!fluids.contains(fluidName))
        return;

    const Fluid &fluid = fluids[fluidName];

    ui->editFluidName->setText(fluid.getName());
    ui->editFluidDescription->setPlainText(fluid.getDescription());

    ui->tableWidgetDensity->setRowCount(0);
    QVector<DensityPoint> denistyTable = fluid.getDensityTableVector();
    for(const auto &entry : denistyTable)
    {
        int row = ui->tableWidgetDensity->rowCount();
        ui->tableWidgetDensity->insertRow(row);
        ui->tableWidgetDensity->setItem(row, 0, new QTableWidgetItem(QString::number(entry.temperature, 'f', 2)));
        ui->tableWidgetDensity->setItem(row, 1, new QTableWidgetItem(QString::number(entry.density, 'f', 5)));
    }
}

void FluidTablesDialog::clearFluidDetails()
{
    ui->editFluidName->clear();
    ui->editFluidDescription->clear();
    ui->tableWidgetDensity->setRowCount(0);
    editingFluidName.clear();
    modified = false;
}

QString FluidTablesDialog::getUniqueFluidName()
{
    QString newFluidName = "Nowa ciecz";
    int counter = 1;
    while(fluids.contains(newFluidName))
    {
        newFluidName = QString("Nowa ciecz %1").arg(counter);
        counter++;
    }
    return newFluidName;
}

bool FluidTablesDialog::isUniqueFluidName(const QString &name) const
{
    return fluids.find(name) == fluids.end();
}

QVector<DensityPoint> FluidTablesDialog::getDensityPointsFromTable() const
{
    QVector<DensityPoint> densityPoints;

    int rowCount = ui->tableWidgetDensity->rowCount();
    for(int row = 0; row < rowCount; row++)
    {
        QTableWidgetItem* temperatureItem = ui->tableWidgetDensity->item(row, 0);
        QTableWidgetItem* densityItem = ui->tableWidgetDensity->item(row, 1);

        if (temperatureItem && densityItem && !temperatureItem->text().trimmed().isEmpty() && !densityItem->text().trimmed().isEmpty())
        {
            bool tempOk, densityOk;
            double temperature = temperatureItem->text().toDouble(&tempOk);
            double density = densityItem->text().toDouble(&densityOk);

            if(tempOk && densityOk)
                densityPoints.append(DensityPoint(temperature, density));
        }
    }

    return densityPoints;
}

void FluidTablesDialog::connectSignalsAndSlots()
{
    connect(ui->buttonAddFluid, &QPushButton::clicked, this, &FluidTablesDialog::buttonAddFluidOnClicked);
    connect(ui->buttonEditFluid, &QPushButton::clicked, this, &FluidTablesDialog::buttonEditFluidOnClicked);
    connect(ui->buttonRemoveFluid, &QPushButton::clicked, this, &FluidTablesDialog::buttonRemoveFluidOnClicked);

    connect(ui->buttonSaveFluid, &QPushButton::clicked, this, &FluidTablesDialog::buttonSaveEditFluidOnClicked);
    connect(ui->buttonCancelEditFluid, &QPushButton::clicked, this, &FluidTablesDialog::buttonCancelEditFluidOnClicked);

    connect(ui->buttonAddRow, &QPushButton::clicked, this, &FluidTablesDialog::buttonAddRowOnClicked);
    connect(ui->buttonRemoveRow, &QPushButton::clicked, this, &FluidTablesDialog::buttonRemoveRowOnClicked);
    connect(ui->buttonImportCSV, &QPushButton::clicked, this, &FluidTablesDialog::buttonImportCsvOnClicked);

    connect(ui->buttonClose, &QPushButton::clicked, this, &FluidTablesDialog::buttonCloseOnClicked);

    connect(ui->listWidgetFluids, &QListWidget::currentItemChanged, this, &FluidTablesDialog::listWidgetFluidsOnCurrentItemChanged);
}

void FluidTablesDialog::connectSignalsForModification()
{
    if(modified)
        return;

    connect(ui->editFluidName, &QLineEdit::textChanged, [this]() {
        modified = ui->groupBoxFluidDetails->isEnabled() && true;
    });

    connect(ui->editFluidDescription, &QPlainTextEdit::textChanged, [this]() {
        modified = ui->groupBoxFluidDetails->isEnabled() && true;
    });

    connect(ui->tableWidgetDensity, &QTableWidget::cellChanged, [this](int row, int column) {
        modified = ui->groupBoxFluidDetails->isEnabled() && true;
    });

    connect(ui->tableWidgetDensity, &QTableWidget::itemChanged, [this](QTableWidgetItem *item) {
        modified = ui->groupBoxFluidDetails->isEnabled() && true;
    });
}

void FluidTablesDialog::updateButtonsState()
{
    ui->groupBoxFluidList->setEnabled(!modified);
}

void FluidTablesDialog::buttonCloseOnClicked()
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycje"), QMessageBox::Ok);
        return;
    }
    // saveFluids();
    accept();
}

void FluidTablesDialog::listWidgetFluidsOnCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(modified)
    {
        QMessageBox::warning(this, tr("Niezapisane zmiany"), tr("Masz niezapisane zmiany. Zapisz lub anuluj edycje"), QMessageBox::Ok);
        return;
    }
    ui->groupBoxFluidDetails->setEnabled(false);
    updateFluidDetails(current->text());
    updateButtonsState();
}

void FluidTablesDialog::updateFluidList()
{
    for(auto fluid = fluids.begin(); fluid != fluids.end(); fluid++)
        ui->listWidgetFluids->addItem(fluid->getName());
    if(ui->listWidgetFluids->count() > 0)
        ui->listWidgetFluids->setCurrentRow(0);
}

void FluidTablesDialog::loadFluids()
{
    QSettings settings;
    QString filePath = settings.value("FluidTablesPath", QCoreApplication::applicationDirPath() + "/fluids.json").toString();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    file.close();

    if (doc.isArray()) {
        QJsonArray fluidsArray = doc.array();

        for (int i = 0; i < fluidsArray.size(); ++i) {
            QJsonObject fluidObj = fluidsArray[i].toObject();

            QJsonArray densityArray = fluidObj["densityTable"].toArray();
            QVector<DensityPoint> densityTable;
            for (int j = 0; j < densityArray.size(); ++j)
            {
                QJsonObject entry = densityArray[j].toObject();
                double temp = entry["temperature"].toDouble();
                double density = entry["density"].toDouble();
                densityTable.push_back(DensityPoint(temp, density));
            }
            Fluid fluid(fluidObj["name"].toString(), fluidObj["description"].toString(), densityTable);

            fluids[fluid.getName()] = fluid;
            ui->listWidgetFluids->addItem(fluid.getName());
        }
    }

    if(ui->listWidgetFluids->count() > 0)
        ui->listWidgetFluids->setCurrentRow(0);
}

void FluidTablesDialog::saveFluids()
{
    QJsonArray fluidsArray;

    for(auto it = fluids.begin(); it != fluids.end(); it++)
    {
        Fluid fluid = *it;
        QJsonObject fluidObj;
        fluidObj["name"] = fluid.getName();
        fluidObj["description"] = fluid.getDescription();

        QJsonArray densityArray;
        QVector<DensityPoint> denistyTable = fluid.getDensityTableVector();
        for(const auto &entry : denistyTable)
        {
            QJsonObject entryObj;
            entryObj["temperature"] = entry.temperature;
            entryObj["density"] = entry.density;
            densityArray.append(entryObj);
        }

        fluidObj["densityTable"] = densityArray;
        fluidsArray.append(fluidObj);
    }

    QJsonDocument doc(fluidsArray);

    QSettings settings;
    QString filePath = settings.value("FluidTablesPath", QCoreApplication::applicationDirPath() + "/fluids.json").toString();

    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
    }
    else
        QMessageBox::warning(this, "Błąd zapisu", "Nie można zapisać danych cieczy do pliku.");
}
