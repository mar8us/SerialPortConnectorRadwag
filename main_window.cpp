#include "main_window.h"
#include "./ui_main_window.h"
#include "fluid_tabels/fluid_tables_form.h"
#include "material_tabels/material_tabels_dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , devicesListModel(this)
    , devicesListControler(devicesListModel, this)
    , materialManager(std::make_unique<MaterialManager>(new MaterialManager(this)))
{
    initControls();
    connectButtons();
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAddDeviceButtonClicked()
{
    devicesListControler.beginNew();
    updateDevicesComboConnection();
}

void MainWindow::onEditDeviceButtonClicked()
{
    devicesListControler.beginEdit(getSelectedDevice());
    updateDevicesComboConnection();
}

void MainWindow::onRemoveDeviceButtonClicked()
{
    devicesListControler.beginRemove(getSelectedDevice());
    updateDevicesComboConnection();
}

void MainWindow::onDeviceComboSelectionChanged()
{
    deviceConnector.setActiveDevice(ui->devicesComboConnection->currentData().value<std::shared_ptr<const Device>>());
}

void MainWindow::onConnectDeviceClicked()
{
    deviceConnector.connectDevice(ui->serialPortCombo->currentText());
}

void MainWindow::navigateToToolBoxPage(QWidget* page)
{
    if(page && ui->stackedWidget->indexOf(page) != -1)
        ui->stackedWidget->setCurrentWidget(page);
}

// void MainWindow::goToPreviousMeasureStage()
// {
//     MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
//     int prevIndex = static_cast<int>(currentStage) - 1;

//     if (prevIndex >= 0)
//     {
//         MeasurementStage prevStage = static_cast<MeasurementStage>(prevIndex);
//         ui->measureDensityStage->setCurrentIndex(prevIndex);
//         ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
//         updateStageLabels();
//     }
// }

// void MainWindow::goToNextMeasureStage()
// {
//     MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
//     int nextIndex = static_cast<int>(currentStage) + 1;

//     if (nextIndex < ui->measureDensityStage->count())
//     {
//         MeasurementStage nextStage = static_cast<MeasurementStage>(nextIndex);
//         ui->measureDensityStage->setCurrentIndex(nextIndex);
//         ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(nextStage));
//         updateStageLabels();
//     }
// }

void MainWindow::goToPreviousMeasureStage()
{
    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    // Określenie poprzedniego etapu pomiaru
    MeasurementStage prevStage;
    switch (currentStage)
    {
    case MeasurementStage::DryMeasure:
        prevStage = MeasurementStage::InitialData;
        ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
        break;

    case MeasurementStage::PrepareSecond:
    case MeasurementStage::PrepareTriple:
        prevStage = MeasurementStage::DryMeasure;
        ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
        break;

    case MeasurementStage::FinishSecond:
        prevStage = MeasurementStage::PrepareSecond;
        ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
        break;

    case MeasurementStage::SaturatedMass:
        prevStage = MeasurementStage::PrepareTriple;
        ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
        break;

    case MeasurementStage::FinishTriple:
        prevStage = MeasurementStage::SaturatedMass;
        ui->measureDensityStage->setCurrentWidget(ui->pageMeasureTriple);
        break;

    default:
        // Dla pierwszego etapu nie ma już poprzedniej strony
        return;
    }

    // Zapisanie aktualnego etapu pomiaru
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
    updateStageLabels();
}

void MainWindow::goToNextMeasureStage()
{
    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    // Określenie następnego etapu pomiaru
    MeasurementStage nextStage;
    switch (currentStage)
    {
    case MeasurementStage::InitialData:
        nextStage = MeasurementStage::DryMeasure;
        ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
        break;

    case MeasurementStage::DryMeasure:
        if (isTripleMeasurement) {
            nextStage = MeasurementStage::PrepareTriple;
            ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
        } else {
            nextStage = MeasurementStage::PrepareSecond;
            ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
        }
        break;

    case MeasurementStage::PrepareSecond:
        nextStage = MeasurementStage::FinishSecond;
        ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementSecond);
        break;

    case MeasurementStage::PrepareTriple:
        nextStage = MeasurementStage::SaturatedMass;
        ui->measureDensityStage->setCurrentWidget(ui->pageMeasureTriple);
        break;

    case MeasurementStage::SaturatedMass:
        nextStage = MeasurementStage::FinishTriple;
        ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementTriple);
        break;

    default:
        // Dla ostatnich etapów nie ma już następnej strony
        return;
    }

    // Zapisanie aktualnego etapu pomiaru
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(nextStage));
    updateStageLabels();
}

void MainWindow::onMeasurementTypeChanged()
{
    // Aktualizacja panelu bocznego z etapami pomiaru
    if (ui->radioMeasureSecond->isChecked()) {
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageSecondMeasure);
    } else {
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageTripleMeasure);
    }

    // Resetowanie etapu pomiaru do strony początkowej
    ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStage::InitialData));
    updateStageLabels();
}

void MainWindow::updateActionIcons(int index)
{
    ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
    ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
}

void MainWindow::buttonTableFluidsOnClicked()
{
    auto dialog = new FluidTablesDialog(this);
    dialog->exec();
}

void MainWindow::buttonTableMatrialsOnClicked()
{
    QMap<QString, Material> materials = materialManager->getMaterials();
    auto dialog = new MaterialTablesDialog(materials, this);
    dialog->exec();
    materialManager->setMaterials(materials);
}

std::shared_ptr<const Device> MainWindow::getSelectedDevice()
{
    QModelIndex currentIndex = ui->devicesListView->currentIndex();
    if (!currentIndex.isValid())
        return nullptr;

    DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
    if (!model)
        return nullptr;

    return model->getDevice(currentIndex.row());
}

void MainWindow::initControls()
{
    ui->setupUi(this);
    setIcons();
    setProperty();
    navigateToToolBoxPage(ui->measureDensityPage);
    updateStageLabels();
    updateActionIcons(0);
    ui->devicesListView->setModel(&devicesListModel);
    updateDevicesComboConnection();
    fillSerialPortCombo();
    onMeasurementTypeChanged();
}

void MainWindow::connectButtons()
{
    connect(ui->actionSettings, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->settingsPage);
    });

    connect(ui->actionMeasureDensity, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->measureDensityPage);
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::updateActionIcons);

    // Połączenie przycisków nawigacji
    connect(ui->buttonNextData, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);

    connect(ui->buttonNextDryMass, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevDryMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    // Przyciski dla pomiaru dwustopniowego
    connect(ui->buttonNextPreparation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevPreparation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
    connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    // Przyciski dla pomiaru trzystopniowego
    connect(ui->buttonNextSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->buttonNextSaturatedMass, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevSaturatedMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->buttonFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
    connect(ui->buttonPrevSaturatedFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    // Przyciski wyboru typu pomiaru
    connect(ui->radioMeasureSecond, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);
    connect(ui->radioMeasureTriple, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);

    // connect(ui->buttonNextData, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    // connect(ui->buttonNextDryMass, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    // connect(ui->buttonPrevDryMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    // connect(ui->buttonNextPreparation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    // connect(ui->buttonPrevPreparation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    // connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    // connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    // connect(ui->buttonPrevSaturatedFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->addDeviceButton, &QPushButton::clicked, this, &MainWindow::onAddDeviceButtonClicked);
    connect(ui->editDeviceButton, &QPushButton::clicked, this, &MainWindow::onEditDeviceButtonClicked);
    connect(ui->deleteDeviceButton, &QPushButton::clicked, this, &MainWindow::onRemoveDeviceButtonClicked);

    connect(ui->devicesComboConnection, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
    connect(ui->buttonConnectDevice, &QPushButton::clicked, this, &MainWindow::onConnectDeviceClicked);

    connect(ui->buttonTableFluids, &QPushButton::clicked, this, &MainWindow::buttonTableFluidsOnClicked);
    connect(ui->buttonTableMatrials, &QPushButton::clicked, this, &MainWindow::buttonTableMatrialsOnClicked);

}

void MainWindow::updateStageLabels()
{
    QFont normalFont;
    normalFont.setBold(false);
    normalFont.setPixelSize(12);

    QFont boldFont = normalFont;
    boldFont.setBold(true);
    boldFont.setPixelSize(13);

    QPalette normalPalette;
    QPalette activePalette;
    activePalette.setColor(QPalette::WindowText, ACTIVE_LABEL_COLOR);

    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    // Resetowanie wszystkich etykiet do stanu normalnego
    if (isTripleMeasurement)
    {
        // Etykiety dla pomiaru trzystopniowego
        QList<QLabel*> tripleLabels = {
            ui->labelStageDataTriple,
            ui->labelStageDryMassTriple,
            ui->labelStageSaturation,
            ui->labelStageSaturatedMass,
            ui->labelStageSaturatedFluidMass,
            ui->labelStageSummaryTriple
        };

        for (QLabel* label : tripleLabels) {
            label->setFont(normalFont);
            label->setPalette(normalPalette);
        }

        // Podświetlenie aktualnego etapu
        switch (currentStage) {
        case MeasurementStage::InitialData:
            ui->labelStageDataTriple->setFont(boldFont);
            ui->labelStageDataTriple->setPalette(activePalette);
            break;
        case MeasurementStage::DryMeasure:
            ui->labelStageDryMassTriple->setFont(boldFont);
            ui->labelStageDryMassTriple->setPalette(activePalette);
            break;
        case MeasurementStage::PrepareTriple:
            ui->labelStageSaturation->setFont(boldFont);
            ui->labelStageSaturation->setPalette(activePalette);
            break;
        case MeasurementStage::SaturatedMass:
            ui->labelStageSaturatedMass->setFont(boldFont);
            ui->labelStageSaturatedMass->setPalette(activePalette);
            break;
        case MeasurementStage::FinishTriple:
            ui->labelStageSummaryTriple->setFont(boldFont);
            ui->labelStageSummaryTriple->setPalette(activePalette);
            break;
        default:
            break;
        }
    } else {
        // Etykiety dla pomiaru dwustopniowego
        QList<QLabel*> secondLabels = {
            ui->labelStageData,
            ui->labelStageDryMass,
            ui->labelStagePreparation,
            ui->labelStageFluidMass,
            ui->labelStageSummary
        };

        for (QLabel* label : secondLabels) {
            label->setFont(normalFont);
            label->setPalette(normalPalette);
        }

        // Podświetlenie aktualnego etapu
        switch (currentStage) {
        case MeasurementStage::InitialData:
            ui->labelStageData->setFont(boldFont);
            ui->labelStageData->setPalette(activePalette);
            break;
        case MeasurementStage::DryMeasure:
            ui->labelStageDryMass->setFont(boldFont);
            ui->labelStageDryMass->setPalette(activePalette);
            break;
        case MeasurementStage::PrepareSecond:
            ui->labelStagePreparation->setFont(boldFont);
            ui->labelStagePreparation->setPalette(activePalette);
            break;
        case MeasurementStage::FinishSecond:
            ui->labelStageFluidMass->setFont(boldFont);
            ui->labelStageFluidMass->setPalette(activePalette);
            break;
        default:
            break;
        }
    }
}

// void MainWindow::updateStageLabels()
// {
//     QFont normalFont = ui->labelStageData->font();
//     normalFont.setBold(false);
//     normalFont.setPixelSize(12);

//     QFont boldFont = normalFont;
//     boldFont.setBold(true);
//     boldFont.setPixelSize(13);

//     QPalette activePalette;
//     activePalette.setColor(QPalette::WindowText, ACTIVE_LABEL_COLOR);

//     ui->labelStageData->setPalette(QPalette());
//     ui->labelStageDryMass->setPalette(QPalette());
//     ui->labelStagePreparation->setPalette(QPalette());
//     ui->labelStageFluidMass->setPalette(QPalette());
//     // ui->measureAirStageLabel_2->setPalette(QPalette());

//     ui->labelStageData->setFont(normalFont);
//     ui->labelStageDryMass->setFont(normalFont);
//     ui->labelStagePreparation->setFont(normalFont);
//     ui->labelStageFluidMass->setFont(normalFont);
//     // ui->measureAirStageLabel_2->setFont(normalFont);

//     MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
//     switch(currentStage)
//     {
//         case MeasurementStage::Data:
//             ui->labelStageData->setFont(boldFont);
//             ui->labelStageData->setPalette(activePalette);
//             break;

//         case MeasurementStage::AirMeasure:
//             ui->labelStageDryMass->setFont(boldFont);
//             ui->labelStageDryMass->setPalette(activePalette);
//             break;

//         case MeasurementStage::PrepareHydro:
//             ui->labelStagePreparation->setFont(boldFont);
//             ui->labelStagePreparation->setPalette(activePalette);
//             break;

//         case MeasurementStage::HydroMeasure:
//             ui->labelStageFluidMass->setFont(boldFont);
//             ui->labelStageFluidMass->setPalette(activePalette);
//             break;

//         case MeasurementStage::AirEndMeasure:
//             //ui->measureAirStageLabel_2->setFont(boldFont);
//             //ui->measureAirStageLabel_2->setPalette(activePalette);
//             break;
//     }
// }

void MainWindow::finishMeasurement()
{
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    // Tutaj można dodać logikę zapisywania wyników pomiaru

    // Informacja o zakończeniu pomiaru
    QString message = isTripleMeasurement ?
                          tr("Pomiar trzystopniowy zakończony pomyślnie.") :
                          tr("Pomiar dwustopniowy zakończony pomyślnie.");

    QMessageBox::information(this, tr("Pomiar zakończony"), message);

    // Powrót do pierwszej strony
    ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStage::InitialData));
    updateStageLabels();
}

void MainWindow::setProperty()
{
    ui->stackedWidget->setProperty("currentStage", QVariant::fromValue(MeasurementStage::InitialData));
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}

void MainWindow::updateDevicesComboConnection()
{
    DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
    if(!model)
        return;

    ui->devicesComboConnection->blockSignals(true);

    const QList<std::shared_ptr<const Device>>& devicesList = model->getDevicesList();
    QString currentText = ui->devicesComboConnection->currentText();
    ui->devicesComboConnection->clear();

    if(!devicesList.size())
        ui->devicesComboConnection->addItem("", QVariant());

    for(const auto& device : devicesList)
    {
        QVariant deviceData;
        deviceData.setValue(device);
        ui->devicesComboConnection->addItem(device->getName(), deviceData);
    }
    int index = ui->devicesComboConnection->findText(currentText);
    if(index != -1 && !currentText.isEmpty())
        ui->devicesComboConnection->setCurrentIndex(index);
    else
        ui->devicesComboConnection->setCurrentIndex(-1);

    ui->devicesComboConnection->blockSignals(false);
}

void MainWindow::fillSerialPortCombo()
{
    ui->serialPortCombo->addItems(deviceConnector.getAvaiablePorts());
}

