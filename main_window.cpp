#include "main_window.h"
#include "./ui_main_window.h"
#include "fluid_tabels/fluid_tables_form.h"
#include "sample/sample_dialog.h"
#include "tooltip/tooltip_manager.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , devicesListModel(this)
    , devicesListControler(devicesListModel, this)
    , fluidManager(std::make_unique<FluidManager>(new FluidManager()))
    , materialManager(std::make_unique<MaterialManager>(new MaterialManager(this)))
    , sampleManager(std::make_unique<SampleManager>(new SampleManager(this)))
{
    initControls();
    connectButtons();
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

    // tooltip i.e
    TooltipManager& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setGlobalStyle("QToolTip { background-color: #2C3E50; color: white; }");
    tooltipManager.registerImage("info", ":/icons/image.jpg", 424, 424);
    tooltipManager.registerTooltip(ui->buttonDryMassExecuteStepOne, ui->buttonDryMassExecuteStepOne->text(), "Wyzeruj wagę wskazanym na ilustracji przyciskiem", "info", TooltipManager::IMAGE_BOTTOM);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAddDeviceButtonClicked()
{
    devicesListControler.beginNew();
    if(!deviceConnector.connectionIsActive())
        fillDevicesCombo();
}

void MainWindow::onEditDeviceButtonClicked()
{
    auto selectedDeivce = getSelectedDevice();
    if(!canEditDevice(selectedDeivce))
    {
        QMessageBox::warning(this, "Ostrzeżenie", QString("Nie możesz edytować urządzenia z aktywnym połączeniem"));
        return;
    }
    devicesListControler.beginEdit(selectedDeivce);
    if(!deviceConnector.connectionIsActive())
        fillDevicesCombo();
}

void MainWindow::onRemoveDeviceButtonClicked()
{
    auto selectedDeivce = getSelectedDevice();
    if(!canEditDevice(selectedDeivce))
    {
        QMessageBox::warning(this, "Ostrzeżenie", QString("Nie możesz usunąć urządzenia z aktywnym połączeniem"));
        return;
    }
    devicesListControler.beginRemove(selectedDeivce);
    if(!deviceConnector.connectionIsActive())
        fillDevicesCombo();
}

void MainWindow::onDeviceComboSelectionChanged()
{
    deviceConnector.setActiveDevice(ui->comboBoxSelectDevice->currentData().value<std::shared_ptr<const Device>>());
}

void MainWindow::onConnectDeviceClicked()
{
    deviceConnector.connectDevice(ui->comboBoxSelectPort->currentText());
}

void MainWindow::onDisconnectDeviceClicked()
{
    deviceConnector.closeActiveConnection();
    fillDevicesCombo(true);
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

void MainWindow::onMainPageChanged(int index)
{
    updateActionIcons(index);
}

void MainWindow::updateActionIcons(int index)
{
    ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
    ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
}

void MainWindow::buttonTableFluidsOnClicked()
{
    QMap<QString, Fluid> fluids = fluidManager->getFluids();
    auto dialog = new FluidTablesDialog(fluids, this);
    dialog->exec();
    fluidManager->setFluids(fluids);
    fillFluidCombo();
}

void MainWindow::buttonSamplesOnClicked()
{
    QMap<QString, Sample> samples = sampleManager->getSamples();
    QMap<QString, Material> materials = materialManager->getMaterials();
    auto dialog = new SampleDialog(samples, materials, this);
    connect(dialog, &SampleDialog::materialsChanged, this, &MainWindow::onMaterialsChanged);
    dialog->exec();
    sampleManager->setSamples(samples);
    fillSampleCombo();
    upadteSampleEditors();
}

void MainWindow::onMaterialsChanged(const QMap<QString, Material> &materials)
{
    materialManager->setMaterials(materials);
}

void MainWindow::onSampleComboBoxChanged(int index)
{
    upadteSampleEditors();
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
    fillDevicesCombo();
    fillSerialPortCombo();
    fillFluidCombo();
    fillSampleCombo();
    onMeasurementTypeChanged();
    updateStatusConnectionLabel(false);
}

void MainWindow::onConnectResult(bool connected)
{
    updateStatusConnectionLabel(connected);
}

void MainWindow::connectButtons()
{
    connect(ui->actionSettings, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->settingsPage);
    });

    connect(ui->actionMeasureDensity, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->measureDensityPage);
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onMainPageChanged);

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

    connect(ui->comboBoxSelectDevice, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
    connect(ui->buttonConnectDevice, &QPushButton::clicked, this, &MainWindow::onConnectDeviceClicked);
    connect(ui->buttonDisconnectDevice, &QPushButton::clicked, this, &MainWindow::onDisconnectDeviceClicked);

    connect(ui->buttonTableFluids, &QPushButton::clicked, this, &MainWindow::buttonTableFluidsOnClicked);
    connect(ui->buttonSamples, &QPushButton::clicked, this, &MainWindow::buttonSamplesOnClicked);

    connect(ui->comboBoxSampleSelection, &QComboBox::currentIndexChanged, this, &MainWindow::onSampleComboBoxChanged);

    connect(&deviceConnector, &DeviceConnector::connectionResult, this, &MainWindow::onConnectResult);
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
    ui->scrollAreaInitialData->setBackgroundRole(QPalette::Base);
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}

void MainWindow::fillDevicesCombo(bool keepActiveDevice)
{
    DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
    if(!model)
        return;

    ui->comboBoxSelectDevice->blockSignals(true);
    const QList<std::shared_ptr<const Device>>& devicesList = model->getDevicesList();

    QString currentDeviceName;
    if(keepActiveDevice)
        currentDeviceName = ui->comboBoxSelectDevice->currentText();
    ui->comboBoxSelectDevice->clear();

    for(const auto& device : devicesList)
    {
        QVariant deviceData;
        deviceData.setValue(device);
        ui->comboBoxSelectDevice->addItem(device->getName(), deviceData);
    }
    if(currentDeviceName.isEmpty())
        ui->comboBoxSelectDevice->setCurrentIndex(-1);
    else
        ui->comboBoxSelectDevice->setCurrentText(currentDeviceName);
    onDeviceComboSelectionChanged();
    ui->comboBoxSelectDevice->blockSignals(false);
}

void MainWindow::fillSerialPortCombo()
{
    ui->comboBoxSelectPort->addItems(deviceConnector.getAvaiablePorts());
}

void MainWindow::fillFluidCombo()
{
    ui->comboBoxFluid->clear();
    const QMap<QString, Fluid> &fluids = fluidManager->getFluids();
    for(auto &fluid : fluids)
        ui->comboBoxFluid->addItem(fluid.getName());
    ui->comboBoxFluid->setCurrentIndex(-1);
}

void MainWindow::fillSampleCombo()
{
    ui->comboBoxSampleSelection->clear();
    const QMap<QString, Sample> &samples = sampleManager->getSamples();
    for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
    {
        QString id = it.value().getId();
        QString name = it.value().getName();
        ui->comboBoxSampleSelection->addItem(it.value().getName(), it.value().getId());
    }
    ui->comboBoxSampleSelection->setCurrentIndex(-1);
}

void MainWindow::updateStatusConnectionLabel(bool connectionStatus)
{
    if(connectionStatus)
    {
        ui->labelEditStatusConnection->setStyleSheet("color: green; font-weight: bold;");
        ui->labelEditStatusConnection->setText("Połączono");
        ui->comboBoxSelectDevice->setEnabled(false);
    }
    else
    {
        ui->labelEditStatusConnection->setStyleSheet("color: red; font-weight: bold;");
        ui->labelEditStatusConnection->setText("Brak połączenia");
        ui->comboBoxSelectDevice->setEnabled(true);
    }
}

void MainWindow::upadteSampleEditors()
{
    clearSampleEditors();

    QString smapleId = ui->comboBoxSampleSelection->currentData().toString();
    Sample sample = sampleManager->getSample(smapleId);
    if(sample.getId().isEmpty())
        return;

    ui->editSampleId->setText(smapleId);
    ui->editSampleName->setText(sample.getName());
    ui->editMaterial->setText(sample.getMaterialName());
    ui->editMaterialDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 3) +  " g/cm³");
    ui->editSampleDescription->setPlainText(sample.getDescription());
}

void MainWindow::clearSampleEditors()
{
    ui->editSampleId->clear();
    ui->editSampleName->clear();
    ui->editMaterial->clear();
    ui->editMaterialDensity->clear();
    ui->editSampleDescription->clear();
}

bool MainWindow::canEditDevice(std::shared_ptr<const Device> &device)
{
    return !deviceConnector.connectionIsActive() || (deviceConnector.connectionIsActive() && device != deviceConnector.getActiveDevice());
}
