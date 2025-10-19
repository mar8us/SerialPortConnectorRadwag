#include "manual_measurements_handler.h"
#include "../main_window.h"
#include "radwag_measure.h"
#include "../app_core.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

ManualMeasurementsHandler::ManualMeasurementsHandler(MainWindow* mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_ui(mainWindow->getUi())
    , m_connector(appCore.getScaleConnector())
    , m_model(new ManualMeasurementsModel(this))
    , m_autoSave(false)
    , m_pendingMeasure(QByteArray())
{

}

ManualMeasurementsHandler::~ManualMeasurementsHandler()
{

}

void ManualMeasurementsHandler::initialize()
{
    setupTreeView();
    populateDefaultLabels();
    fillComboManualGroupBy();
    setupConnections();

    m_ui->line_4->setVisible(false);
    m_ui->btnCreateHydroMeasurement->setVisible(false);
    m_ui->btnExportExcel->setVisible(false);
    // addTestData();
}

void ManualMeasurementsHandler::setupTreeView()
{
    if(!m_ui || !m_ui->treeViewManualMeasures)
        return;

    m_ui->treeViewManualMeasures->setModel(m_model);
    m_ui->treeViewManualMeasures->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui->treeViewManualMeasures->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::DateTime, 180);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Value, 100);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Unit, 80);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Stability, 90);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Label, 150);
}

void ManualMeasurementsHandler::populateDefaultLabels()
{
    if(!m_ui || !m_ui->listLabels)
        return;

    m_ui->listLabels->addItem("Próbka sucha");
    m_ui->listLabels->addItem("Pomiar w cieczy");
    m_ui->listLabels->addItem("Pomiar w powietrzu");
    m_ui->listLabels->addItem("Próbka nasycona");
}

void ManualMeasurementsHandler::fillComboManualGroupBy()
{
    if(!m_ui || !m_ui->comboManualGroupBy)
        return;

    m_ui->comboManualGroupBy->clear();
    m_ui->comboManualGroupBy->addItem("Brak grupowania", -1);
    m_ui->comboManualGroupBy->addItem("Data", ManualMeasurementsModel::DateTime);
    m_ui->comboManualGroupBy->addItem("Wartość", ManualMeasurementsModel::Value);
    m_ui->comboManualGroupBy->addItem("Jednostka", ManualMeasurementsModel::Unit);
    m_ui->comboManualGroupBy->addItem("Stabilność", ManualMeasurementsModel::Stability);
    m_ui->comboManualGroupBy->addItem("Etykieta", ManualMeasurementsModel::Label);
}

void ManualMeasurementsHandler::setupConnections()
{
    if(!m_ui)
        return;

    connect(m_ui->btnGetMeasure, &QPushButton::clicked, this, &ManualMeasurementsHandler::onGetMeasureClicked);
    connect(m_ui->btnSaveMeasure, &QPushButton::clicked, this, &ManualMeasurementsHandler::onSaveMeasureClicked);
    connect(m_ui->btnAddLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onAddLabelClicked);
    connect(m_ui->btnRemoveLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onRemoveLabelClicked);
    connect(m_ui->btnAssignLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onAssignLabelClicked);
    connect(m_ui->btnDeleteSelected, &QPushButton::clicked, this, &ManualMeasurementsHandler::onDeleteSelectedClicked);
    connect(m_ui->btnClearAll, &QPushButton::clicked, this, &ManualMeasurementsHandler::onClearAllClicked);

    connect(m_ui->checkAutoSave, &QCheckBox::toggled, this, &ManualMeasurementsHandler::onAutoSaveToggled);
    connect(m_ui->comboManualGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManualMeasurementsHandler::onManualGroupByChanged);

    connect(m_connector, &RadwagScaleConnector::radwagDataReady, this, &ManualMeasurementsHandler::onRadwagDataReady);
}

void ManualMeasurementsHandler::addTestData()
{
    ManualMeasurementRecord record1;
    record1.timestamp = QDateTime::currentDateTime().addSecs(-300);
    record1.value = 9.9885;
    record1.unit = "g";
    record1.isStable = true;
    record1.label = "Pomiar w powietrzu";
    m_model->addMeasurement(record1);

    ManualMeasurementRecord record2;
    record2.timestamp = QDateTime::currentDateTime().addSecs(-250);
    record2.value = 9.9880;
    record2.unit = "g";
    record2.isStable = true;
    record2.label = "Pomiar w powietrzu";
    m_model->addMeasurement(record2);

    ManualMeasurementRecord record3;
    record3.timestamp = QDateTime::currentDateTime().addSecs(-200);
    record3.value = 4.3335;
    record3.unit = "g";
    record3.isStable = true;
    record3.label = "Pomiar w wodzie";
    m_model->addMeasurement(record3);

    ManualMeasurementRecord record4;
    record4.timestamp = QDateTime::currentDateTime().addSecs(-150);
    record4.value = 4.3340;
    record4.unit = "g";
    record4.isStable = false;
    record4.label = "Pomiar w wodzie";
    m_model->addMeasurement(record4);

    ManualMeasurementRecord record5;
    record5.timestamp = QDateTime::currentDateTime().addSecs(-100);
    record5.value = 2.1234;
    record5.unit = "g";
    record5.isStable = true;
    record5.label = "";
    m_model->addMeasurement(record5);

    ManualMeasurementRecord record6;
    record6.timestamp = QDateTime::currentDateTime().addSecs(-50);
    record6.value = 10.5678;
    record6.unit = "g";
    record6.isStable = true;
    record6.label = "Próbka nasycona";
    m_model->addMeasurement(record6);
}

void ManualMeasurementsHandler::updateCurrentValueDisplay(const RadwagMeasure& measure)
{
    if(!m_ui)
        return;

    QString valueText = QString("%1 %2").arg(measure.getValue(), 0, 'f', 4).arg(measure.getUnitString());
    m_ui->lblCurrentValue->setText(valueText);
}

void ManualMeasurementsHandler::onGetMeasureClicked()
{
    if(!m_connector)
        return;
    m_connector->sendImmediateWeightCommand();
}

void ManualMeasurementsHandler::onSaveMeasureClicked()
{
    if(m_pendingMeasure.getData().isEmpty())
    {
        QMessageBox::warning(m_mainWindow, "Błąd", "Brak pomiaru");
        return;
    }

    ManualMeasurementRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.value = m_pendingMeasure.getValue();
    record.unit = m_pendingMeasure.getUnitString();
    record.isStable = m_pendingMeasure.isStable();
    record.label = "";
    record.rawData = m_pendingMeasure.getData();

    m_model->addMeasurement(record);

    qDebug() << "Zapisano pomiar:" << record.value << record.unit
             << "Stabilny:" << (record.isStable ? "TAK" : "NIE");
}

void ManualMeasurementsHandler::onAddLabelClicked()
{
    if(!m_ui || !m_ui->editNewLabel || !m_ui->listLabels)
        return;

    QString newLabel = m_ui->editNewLabel->text().trimmed();
    if(newLabel.isEmpty())
    {
        QMessageBox::warning(m_mainWindow, "Błąd", "Nazwa etykiety nie może być pusta");
        return;
    }

    for(int i = 0; i < m_ui->listLabels->count(); i++)
    {
        if(m_ui->listLabels->item(i)->text() == newLabel)
        {
            QMessageBox::warning(m_mainWindow, "Błąd", "Etykieta o tej nazwie już istnieje");
            return;
        }
    }

    m_ui->listLabels->addItem(newLabel);
    m_ui->editNewLabel->clear();
}

void ManualMeasurementsHandler::onRemoveLabelClicked()
{
    if(!m_ui || !m_ui->listLabels)
        return;

    QListWidgetItem* currentItem = m_ui->listLabels->currentItem();
    if(!currentItem)
    {
        QMessageBox::warning(m_mainWindow, "Błąd", "Wybierz etykietę do usunięcia");
        return;
    }

    delete currentItem;
}

void ManualMeasurementsHandler::onAssignLabelClicked()
{
    if(!m_ui || !m_ui->treeViewManualMeasures || !m_ui->listLabels)
        return;

    QModelIndexList selectedMeasurements = m_ui->treeViewManualMeasures->selectionModel()->selectedRows();

    if(selectedMeasurements.isEmpty())
        return;

    QListWidgetItem* selectedLabelItem = m_ui->listLabels->currentItem();
    if(!selectedLabelItem)
    {
        QMessageBox::warning(m_mainWindow, "Uwaga", "Nie wybrano etykiety do przypisania");
        return;
    }

    QString labelText = selectedLabelItem->text();
    int measurementsCount = m_model->countMeasurementsInIndexes(selectedMeasurements);

    QString message = QString("Czy na pewno chcesz przypisać etykietę \"%1\" do %2 %3?")
        .arg(labelText)
        .arg(measurementsCount)
        .arg(measurementsCount == 1 ? "pomiaru" : (measurementsCount < 5 ? "pomiarów" : "pomiarów"));

    QMessageBox::StandardButton reply = QMessageBox::question(
        m_mainWindow,
        "Potwierdzenie przypisania etykiety",
        message,
        QMessageBox::Yes | QMessageBox::No
    );

    if(reply == QMessageBox::Yes)
        m_model->assignLabelToIndexes(selectedMeasurements, labelText);
}

void ManualMeasurementsHandler::onDeleteSelectedClicked()
{
    if(!m_ui || !m_ui->treeViewManualMeasures)
        return;

    QModelIndexList selectedIndexes = m_ui->treeViewManualMeasures->selectionModel()->selectedRows();

    if(selectedIndexes.isEmpty())
    {
        QMessageBox::warning(m_mainWindow, "Uwaga", "Nie zaznaczono żadnych pomiarów do usunięcia");
        return;
    }

    int measurementsCount = m_model->countMeasurementsInIndexes(selectedIndexes);

    QString message = QString("Czy na pewno chcesz usunąć %1 %2?")
        .arg(measurementsCount)
        .arg(measurementsCount == 1 ? "pomiar" : (measurementsCount < 5 ? "pomiary" : "pomiarów"));

    QMessageBox::StandardButton reply = QMessageBox::question(
        m_mainWindow,
        "Potwierdzenie usunięcia",
        message,
        QMessageBox::Yes | QMessageBox::No
    );

    if(reply == QMessageBox::Yes)
        m_model->removeMeasurements(selectedIndexes);
}

void ManualMeasurementsHandler::onClearAllClicked()
{
    if(!m_ui)
        return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        m_mainWindow,
        "Potwierdzenie",
        "Czy na pewno chcesz usunąć wszystkie pomiary?",
        QMessageBox::Yes | QMessageBox::No
    );

    if(reply == QMessageBox::Yes)
    {
        m_model->clearAll();
    }
}

void ManualMeasurementsHandler::onAutoSaveToggled(bool checked)
{
    m_autoSave = checked;
    qDebug() << "Auto-save:" << (checked ? "włączony" : "wyłączony");
}

void ManualMeasurementsHandler::onManualGroupByChanged(int index)
{
    if(!m_ui || !m_ui->comboManualGroupBy)
        return;

    int columnEnum = m_ui->comboManualGroupBy->itemData(index).toInt();
    m_model->setGroupBy(columnEnum);
    m_ui->treeViewManualMeasures->expandAll();

    qDebug() << "Grupowanie zmienione na kolumnę:" << columnEnum;
}

void ManualMeasurementsHandler::onRadwagDataReady(const RadwagMeasure& measure)
{
    m_pendingMeasure = measure;
    updateCurrentValueDisplay(measure);

    if(m_autoSave)
    {
        ManualMeasurementRecord record;
        record.timestamp = QDateTime::currentDateTime();
        record.value = measure.getValue();
        record.unit = measure.getUnitString();
        record.isStable = measure.isStable();
        record.label = "";
        record.rawData = measure.getData();

        m_model->addMeasurement(record);
    }
}

void ManualMeasurementsHandler::updateCurrentMeasureButtonsState(bool enable)
{
    m_ui->btnGetMeasure->setEnabled(enable);
    m_ui->btnSaveMeasure->setEnabled(enable);
}
