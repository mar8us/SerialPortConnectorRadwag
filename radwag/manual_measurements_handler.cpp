#include "manual_measurements_handler.h"
#include "../main_window.h"
#include "radwag_measure.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

ManualMeasurementsHandler::ManualMeasurementsHandler(MainWindow* mainWindow,
                                                     const RadwagScaleConnector* connector,
                                                     QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_ui(mainWindow->getUi())
    , m_connector(connector)
    , m_model(new ManualMeasurementsModel(this))
    , m_isRecording(false)
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
    addTestData();

    if(m_ui)
    {
        m_ui->btnGetMeasure->setEnabled(false);
        m_ui->btnSaveMeasure->setEnabled(false);
    }
}

void ManualMeasurementsHandler::setupTreeView()
{
    if(!m_ui || !m_ui->treeViewManualMeasures)
        return;

    m_ui->treeViewManualMeasures->setModel(m_model);
    m_ui->treeViewManualMeasures->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui->treeViewManualMeasures->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Dostosuj szerokość kolumn
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::DateTime, 180);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Value, 100);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Unit, 80);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Stability, 90);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Label, 150);
    m_ui->treeViewManualMeasures->setColumnWidth(ManualMeasurementsModel::Note, 200);
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
    m_ui->comboManualGroupBy->addItem("Notatka", ManualMeasurementsModel::Note);
}

void ManualMeasurementsHandler::setupConnections()
{
    if(!m_ui)
        return;

    // Przyciski
    connect(m_ui->btnToggleRecording, &QPushButton::clicked, this, &ManualMeasurementsHandler::onToggleRecordingClicked);
    connect(m_ui->btnGetMeasure, &QPushButton::clicked, this, &ManualMeasurementsHandler::onGetMeasureClicked);
    connect(m_ui->btnSaveMeasure, &QPushButton::clicked, this, &ManualMeasurementsHandler::onSaveMeasureClicked);
    connect(m_ui->btnAddLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onAddLabelClicked);
    connect(m_ui->btnRemoveLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onRemoveLabelClicked);
    connect(m_ui->btnAssignLabel, &QPushButton::clicked, this, &ManualMeasurementsHandler::onAssignLabelClicked);
    connect(m_ui->btnDeleteSelected, &QPushButton::clicked, this, &ManualMeasurementsHandler::onDeleteSelectedClicked);
    connect(m_ui->btnClearAll, &QPushButton::clicked, this, &ManualMeasurementsHandler::onClearAllClicked);

    // Checkboxy i ComboBox
    connect(m_ui->checkAutoSave, &QCheckBox::toggled, this, &ManualMeasurementsHandler::onAutoSaveToggled);
    connect(m_ui->comboManualGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ManualMeasurementsHandler::onManualGroupByChanged);

    // Sygnał z wagi - zawsze podłączony aby odbierać odpowiedzi na komendę SI
    connect(m_connector, &RadwagScaleConnector::radwagDataReady,
            this, &ManualMeasurementsHandler::onRadwagDataReady);
}

void ManualMeasurementsHandler::addTestData()
{
    ManualMeasurementRecord record1;
    record1.timestamp = QDateTime::currentDateTime().addSecs(-300);
    record1.value = 9.9885;
    record1.unit = "g";
    record1.isStable = true;
    record1.label = "Pomiar w powietrzu";
    record1.note = "";
    m_model->addMeasurement(record1);

    ManualMeasurementRecord record2;
    record2.timestamp = QDateTime::currentDateTime().addSecs(-250);
    record2.value = 9.9880;
    record2.unit = "g";
    record2.isStable = true;
    record2.label = "Pomiar w powietrzu";
    record2.note = "";
    m_model->addMeasurement(record2);

    ManualMeasurementRecord record3;
    record3.timestamp = QDateTime::currentDateTime().addSecs(-200);
    record3.value = 4.3335;
    record3.unit = "g";
    record3.isStable = true;
    record3.label = "Pomiar w wodzie";
    record3.note = "Temp: 20°C";
    m_model->addMeasurement(record3);

    ManualMeasurementRecord record4;
    record4.timestamp = QDateTime::currentDateTime().addSecs(-150);
    record4.value = 4.3340;
    record4.unit = "g";
    record4.isStable = false;
    record4.label = "Pomiar w wodzie";
    record4.note = "";
    m_model->addMeasurement(record4);

    ManualMeasurementRecord record5;
    record5.timestamp = QDateTime::currentDateTime().addSecs(-100);
    record5.value = 2.1234;
    record5.unit = "g";
    record5.isStable = true;
    record5.label = "";
    record5.note = "";
    m_model->addMeasurement(record5);

    ManualMeasurementRecord record6;
    record6.timestamp = QDateTime::currentDateTime().addSecs(-50);
    record6.value = 10.5678;
    record6.unit = "g";
    record6.isStable = true;
    record6.label = "Próbka nasycona";
    record6.note = "Test automatyczny";
    m_model->addMeasurement(record6);
}

void ManualMeasurementsHandler::updateCurrentValueDisplay(const RadwagMeasure& measure)
{
    if(!m_ui)
        return;

    // Aktualizuj wyświetlanie aktualnej wartości
    QString valueText = QString("%1 %2")
        .arg(measure.getValue(), 0, 'f', 4)
        .arg(measure.getUnitString());
    m_ui->lblCurrentValue->setText(valueText);

    // Aktualizuj stabilność
    QString stabilityText = measure.isStable() ? "TAK" : "NIE";
    m_ui->lblStability->setText(stabilityText);

    // Zmień kolor w zależności od stabilności
    if(measure.isStable())
        m_ui->lblStability->setStyleSheet("color: green; font-weight: bold;");
    else
        m_ui->lblStability->setStyleSheet("color: red; font-weight: bold;");
}

// Slots
void ManualMeasurementsHandler::onToggleRecordingClicked(bool checked)
{
    if(!m_ui)
        return;

    m_isRecording = checked;

    if(checked)
    {
        m_ui->btnToggleRecording->setText("Wyłącz rejestrowanie");
        m_ui->btnGetMeasure->setEnabled(true);
        m_ui->btnSaveMeasure->setEnabled(true);
        qDebug() << "Rejestrowanie włączone - przyciski aktywne";
    }
    else
    {
        m_ui->btnToggleRecording->setText("Włącz rejestrowanie");
        m_ui->btnGetMeasure->setEnabled(false);
        m_ui->btnSaveMeasure->setEnabled(false);
        qDebug() << "Rejestrowanie wyłączone - przyciski nieaktywne";
    }
}

void ManualMeasurementsHandler::onGetMeasureClicked()
{
    if(!m_connector)
        return;

    // Wyślij komendę SI do wagi aby pobrać aktualny pomiar
    m_connector->sendCommand("SI");

    qDebug() << "Wysłano komendę SI - pobieranie pomiaru z wagi";
}

void ManualMeasurementsHandler::onSaveMeasureClicked()
{
    // Sprawdź czy mamy pobrany pomiar
    if(m_pendingMeasure.getData().isEmpty())
    {
        QMessageBox::warning(m_mainWindow, "Błąd", "Najpierw pobierz pomiar z wagi");
        return;
    }

    ManualMeasurementRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.value = m_pendingMeasure.getValue();
    record.unit = m_pendingMeasure.getUnitString();
    record.isStable = m_pendingMeasure.isStable();
    record.label = "";  // User może przypisać etykietę później
    record.note = "";
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
    QMessageBox::information(m_mainWindow, "Info", "Funkcja przypisywania etykiet będzie wkrótce zaimplementowana");
}

void ManualMeasurementsHandler::onDeleteSelectedClicked()
{
    QMessageBox::information(m_mainWindow, "Info", "Funkcja usuwania pomiarów będzie wkrótce zaimplementowana");
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
    // Zapisz pobrany pomiar jako oczekujący
    m_pendingMeasure = measure;

    // Zawsze aktualizuj wyświetlanie aktualnej wartości
    updateCurrentValueDisplay(measure);

    qDebug() << "Pobrano pomiar z wagi:" << measure.getValue() << measure.getUnitString()
             << "Stabilny:" << (measure.isStable() ? "TAK" : "NIE");

    // Jeśli auto-save włączony - automatycznie zapisz pomiar
    if(m_autoSave)
    {
        ManualMeasurementRecord record;
        record.timestamp = QDateTime::currentDateTime();
        record.value = measure.getValue();
        record.unit = measure.getUnitString();
        record.isStable = measure.isStable();
        record.label = "";
        record.note = "";
        record.rawData = measure.getData();

        m_model->addMeasurement(record);

        qDebug() << "Auto-save: automatycznie zapisano pomiar" << record.value << record.unit;
    }
}
