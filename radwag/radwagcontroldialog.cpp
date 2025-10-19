#include "radwagcontroldialog.h"
#include "radwag/ui_radwagcontroldialog.h"
#include <qboxlayout.h>
#include <qdatetime.h>
#include <qgroupbox.h>
#include <qpushbutton.h>


RadwagControlDialog::RadwagControlDialog(const RadwagScaleConnector* connector, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadwagControlDialog)
    , m_connector(connector)
{
    ui->setupUi(this);
    setWindowTitle("Sterowanie Wagą Radwag");
    setMinimumSize(600, 700);

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setupUI();

    if(m_connector)
    {
        connect(m_connector, &RadwagScaleConnector::radwagDataReady,
                this, [this](const RadwagMeasure& measure) {
                    QString text = QString("%1 %2 %3")
                    .arg(measure.getValue(), 0, 'f', 4)
                        .arg(measure.getUnitString())
                        .arg(measure.isStable() ? "[STABILNY]" : "[NIESTABILNY]");
                    onMeasurementReceived(text);
                });
    }
}

RadwagControlDialog::~RadwagControlDialog()
{
    delete ui;
}

void RadwagControlDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // === GRUPA: Kalibracja ===
    QGroupBox* groupCalibration = new QGroupBox("Kalibracja i Zerowanie");
    QHBoxLayout* layoutCalibration = new QHBoxLayout(groupCalibration);

    m_btnTare = new QPushButton("TARE\n(Taruj)");
    m_btnTare->setMinimumHeight(60);
    m_btnTare->setStyleSheet("QPushButton { font-size: 12pt; font-weight: bold; }");
    connect(m_btnTare, &QPushButton::clicked, this, &RadwagControlDialog::onTareClicked);

    m_btnZero = new QPushButton("ZERO\n(Wyzeruj)");
    m_btnZero->setMinimumHeight(60);
    m_btnZero->setStyleSheet("QPushButton { font-size: 12pt; font-weight: bold; }");
    connect(m_btnZero, &QPushButton::clicked, this, &RadwagControlDialog::onZeroClicked);

    layoutCalibration->addWidget(m_btnTare);
    layoutCalibration->addWidget(m_btnZero);

    // === GRUPA: Pomiary Jednorazowe - Jednostka Podstawowa ===
    QGroupBox* groupSingleBasic = new QGroupBox("Pomiary Jednorazowe (Jednostka Podstawowa)");
    QVBoxLayout* layoutSingleBasic = new QVBoxLayout(groupSingleBasic);

    m_btnImmediateWeight = new QPushButton("Pomiar Natychmiastowy");
    m_btnImmediateWeight->setMinimumHeight(40);
    connect(m_btnImmediateWeight, &QPushButton::clicked, this, &RadwagControlDialog::onImmediateWeightClicked);

    m_btnStableWeight = new QPushButton("Pomiar Stabilny");
    m_btnStableWeight->setMinimumHeight(40);
    connect(m_btnStableWeight, &QPushButton::clicked, this, &RadwagControlDialog::onStableWeightClicked);

    layoutSingleBasic->addWidget(m_btnImmediateWeight);
    layoutSingleBasic->addWidget(m_btnStableWeight);

    // === GRUPA: Pomiary Jednorazowe - Jednostka Bieżąca ===
    QGroupBox* groupSingleCurrent = new QGroupBox("Pomiary Jednorazowe (Jednostka Bieżąca)");
    QVBoxLayout* layoutSingleCurrent = new QVBoxLayout(groupSingleCurrent);

    m_btnImmediateWeightCurrentUnit = new QPushButton("Pomiar Natychmiastowy (Bieżąca Jednostka)");
    m_btnImmediateWeightCurrentUnit->setMinimumHeight(40);
    connect(m_btnImmediateWeightCurrentUnit, &QPushButton::clicked,
            this, &RadwagControlDialog::onImmediateWeightCurrentUnitClicked);

    m_btnStableWeightCurrentUnit = new QPushButton("Pomiar Stabilny (Bieżąca Jednostka)");
    m_btnStableWeightCurrentUnit->setMinimumHeight(40);
    connect(m_btnStableWeightCurrentUnit, &QPushButton::clicked,
            this, &RadwagControlDialog::onStableWeightCurrentUnitClicked);

    layoutSingleCurrent->addWidget(m_btnImmediateWeightCurrentUnit);
    layoutSingleCurrent->addWidget(m_btnStableWeightCurrentUnit);

    // === GRUPA: Transmisja Ciągła - Jednostka Podstawowa ===
    QGroupBox* groupContinuousBasic = new QGroupBox("Transmisja Ciągła (Jednostka Podstawowa)");
    QHBoxLayout* layoutContinuousBasic = new QHBoxLayout(groupContinuousBasic);

    m_btnStartContinuousBasic = new QPushButton("START\nTransmisja Ciągła");
    m_btnStartContinuousBasic->setMinimumHeight(60);
    m_btnStartContinuousBasic->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(m_btnStartContinuousBasic, &QPushButton::clicked,
            this, &RadwagControlDialog::onStartContinuousBasicClicked);

    m_btnStopContinuousBasic = new QPushButton("STOP\nTransmisja Ciągła");
    m_btnStopContinuousBasic->setMinimumHeight(60);
    m_btnStopContinuousBasic->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    connect(m_btnStopContinuousBasic, &QPushButton::clicked,
            this, &RadwagControlDialog::onStopContinuousBasicClicked);

    layoutContinuousBasic->addWidget(m_btnStartContinuousBasic);
    layoutContinuousBasic->addWidget(m_btnStopContinuousBasic);

    // === GRUPA: Transmisja Ciągła - Jednostka Bieżąca ===
    QGroupBox* groupContinuousCurrent = new QGroupBox("Transmisja Ciągła (Jednostka Bieżąca)");
    QHBoxLayout* layoutContinuousCurrent = new QHBoxLayout(groupContinuousCurrent);

    m_btnStartContinuousCurrent = new QPushButton("START\nTransmisja Ciągła\n(Bieżąca Jednostka)");
    m_btnStartContinuousCurrent->setMinimumHeight(60);
    m_btnStartContinuousCurrent->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(m_btnStartContinuousCurrent, &QPushButton::clicked,
            this, &RadwagControlDialog::onStartContinuousCurrentClicked);

    m_btnStopContinuousCurrent = new QPushButton("STOP\nTransmisja Ciągła\n(Bieżąca Jednostka)");
    m_btnStopContinuousCurrent->setMinimumHeight(60);
    m_btnStopContinuousCurrent->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    connect(m_btnStopContinuousCurrent, &QPushButton::clicked,
            this, &RadwagControlDialog::onStopContinuousCurrentClicked);

    layoutContinuousCurrent->addWidget(m_btnStartContinuousCurrent);
    layoutContinuousCurrent->addWidget(m_btnStopContinuousCurrent);

    // === LOG ===
    QLabel* lblLogHeader = new QLabel("Log Komend i Pomiarów:");
    m_txtLog = new QTextEdit();
    m_txtLog->setReadOnly(true);
    m_txtLog->setMaximumHeight(150);
    QFont logFont("Courier New", 9);
    m_txtLog->setFont(logFont);

    // === PRZYCISK ZAMKNIJ ===
    QPushButton* btnClose = new QPushButton("Zamknij");
    btnClose->setMinimumHeight(40);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    // === DODAJ DO GŁÓWNEGO LAYOUTU ===
    mainLayout->addWidget(groupCalibration);
    mainLayout->addWidget(groupSingleBasic);
    mainLayout->addWidget(groupSingleCurrent);
    mainLayout->addWidget(groupContinuousBasic);
    mainLayout->addWidget(groupContinuousCurrent);
    mainLayout->addWidget(lblLogHeader);
    mainLayout->addWidget(m_txtLog);
    mainLayout->addWidget(btnClose);
}

void RadwagControlDialog::logCommand(const QString& commandName, bool success)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString status = success ? "[OK]" : "[BŁĄD]";
    QString color = success ? "green" : "red";

    QString logEntry = QString("<span style='color: gray;'>%1</span> "
                               "<span style='color: %2; font-weight: bold;'>%3</span> "
                               "<span style='color: blue;'>%4</span>")
                           .arg(timestamp)
                           .arg(color)
                           .arg(status)
                           .arg(commandName);

    m_txtLog->append(logEntry);
}

void RadwagControlDialog::onMeasurementReceived(const QString& measurement)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString logEntry = QString("<span style='color: gray;'>%1</span> "
                               "<span style='color: purple; font-weight: bold;'>[POMIAR]</span> "
                               "<span>%2</span>")
                           .arg(timestamp)
                           .arg(measurement);

    m_txtLog->append(logEntry);
}

// === IMPLEMENTACJA SLOTÓW ===

void RadwagControlDialog::onTareClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendTareCommand();
    logCommand("TARE (Tarowanie)", success);
}

void RadwagControlDialog::onZeroClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendZeroCommand();
    logCommand("ZERO (Zerowanie)", success);
}

void RadwagControlDialog::onImmediateWeightClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendImmediateWeightCommand();
    logCommand("Pomiar Natychmiastowy (Jednostka Podstawowa)", success);
}

void RadwagControlDialog::onStableWeightClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendStableWeightCommand();
    logCommand("Pomiar Stabilny (Jednostka Podstawowa)", success);
}

void RadwagControlDialog::onStableWeightCurrentUnitClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendStableWeightCurrentUnitCommand();
    logCommand("Pomiar Stabilny (Jednostka Bieżąca)", success);
}

void RadwagControlDialog::onImmediateWeightCurrentUnitClicked()
{
    if(!m_connector) return;
    bool success = m_connector->sendImmediateWeightCurrentUnitCommand();
    logCommand("Pomiar Natychmiastowy (Jednostka Bieżąca)", success);
}

void RadwagControlDialog::onStartContinuousBasicClicked()
{
    if(!m_connector) return;
    bool success = m_connector->startContinuousTransmissionBasicUnit();
    logCommand("START Transmisja Ciągła (Jednostka Podstawowa)", success);
}

void RadwagControlDialog::onStopContinuousBasicClicked()
{
    if(!m_connector) return;
    bool success = m_connector->stopContinuousTransmissionBasicUnit();
    logCommand("STOP Transmisja Ciągła (Jednostka Podstawowa)", success);
}

void RadwagControlDialog::onStartContinuousCurrentClicked()
{
    if(!m_connector) return;
    bool success = m_connector->startContinuousTransmissionCurrentUnit();
    logCommand("START Transmisja Ciągła (Jednostka Bieżąca)", success);
}

void RadwagControlDialog::onStopContinuousCurrentClicked()
{
    if(!m_connector) return;
    bool success = m_connector->stopContinuousTransmissionCurrentUnit();
    logCommand("STOP Transmisja Ciągła (Jednostka Bieżąca)", success);
}
