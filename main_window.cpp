#include "main_window.h"
#include "./ui_main_window.h"
#include "fluid_tabels/fluid_tables_form.h"
#include "sample/sample_dialog.h"
#include "tooltip/tooltip_manager.h"
#include <QMessageBox>
#include "radwag/measurement.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , devicesListModel(this)
    , devicesListControler(devicesListModel, this)
    , radwagScaleConnector(nullptr)
    , fluidManager(std::make_unique<FluidManager>(new FluidManager()))
    , materialManager(std::make_unique<MaterialManager>(new MaterialManager(this)))
    , sampleManager(std::make_unique<SampleManager>(new SampleManager(this)))
    , measurementManager(new MeasurementManager(this))
    , radwagMeasureControler(std::make_unique<MeasurementController>(measurementManager, this))
{
    initControls();
    connectButtons();
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

    // tooltip i.e
    TooltipManager& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setGlobalStyle("QToolTip { background-color: #2C3E50; color: white; }");
    tooltipManager.registerImage("info", ":/icons/image.jpg", 424, 424);
    tooltipManager.registerTooltip(ui->buttonDryMassExecuteStepOne, ui->buttonDryMassExecuteStepOne->text(), "Wyzeruj wagę wskazanym na ilustracji przyciskiem", "info", TooltipManager::IMAGE_BOTTOM);

    setupLibraryView();
    connect(ui->treeViewLibMeasure->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//----------------------------------------------- TAB DEVICE MANAGER ------------------------------------------

void MainWindow::onAddDeviceButtonClicked()
{
    devicesListControler.beginNew();
    if(!radwagScaleConnector.get())
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
    if(!radwagScaleConnector.get())
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
    if(!radwagScaleConnector.get())
        fillDevicesCombo();
}

bool MainWindow::canEditDevice(std::shared_ptr<const Device> &device)
{
    return !radwagScaleConnector.get() || (radwagScaleConnector.get() && device != radwagScaleConnector->getActiveDevice());
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

void MainWindow::connectDevice()
{
    radwagScaleConnector.reset(new RadwagScaleConnector());

    connect(radwagScaleConnector.get(), &RadwagScaleConnector::radwagDataReady, this, &MainWindow::onRadwagMeasueReady);
    connect(radwagScaleConnector.get(), &DeviceConnector::connectionResult, this, &MainWindow::onConnectResult);

    radwagScaleConnector->setActiveDevice(ui->comboBoxSelectDevice->currentData().value<std::shared_ptr<const Device>>());
    radwagScaleConnector->connectDevice(ui->comboBoxSelectPort->currentText());
}

void MainWindow::disconnectDevice()
{
    if(!radwagScaleConnector.get())
        return;
    radwagScaleConnector->closeActiveConnection();

    disconnect(radwagScaleConnector.get(), &RadwagScaleConnector::radwagDataReady, this, &MainWindow::onRadwagMeasueReady);
    disconnect(radwagScaleConnector.get(), &DeviceConnector::connectionResult, this, &MainWindow::onConnectResult);

    radwagScaleConnector.reset();
}

bool MainWindow::checkDeviceConnectionWithMessage()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
    {
        QMessageBox::warning(this, tr("Brak połączenia"), "Brak połączenia z urządzeniem");
        return false;
    }
    return true;
}

//----------------------------------------------- END TAB DEVICE MANAGER ------------------------------------------

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
        auto deviceType = device->getDeviceType();
        if(deviceType != DeviceType::RadwagScaleAC220 && deviceType != DeviceType::RadwagScaleAC350)
            continue;

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
    foreach(auto &port, QSerialPortInfo::availablePorts())
    ui->comboBoxSelectPort->addItem(port.portName());
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

void MainWindow::fillFluidCombo()
{
    ui->comboBoxFluid->clear();
    const QMap<QString, Fluid> &fluids = fluidManager->getFluids();
    for(auto &fluid : fluids)
        ui->comboBoxFluid->addItem(fluid.getName());
    ui->comboBoxFluid->setCurrentIndex(-1);
}


void MainWindow::onDeviceComboSelectionChanged()
{
    updateConnectonLabelsStatusBar(false);
}

void MainWindow::onConnectDeviceClicked()
{
    connectDevice();
}

void MainWindow::onDisconnectDeviceClicked()
{
    disconnectDevice();
    fillDevicesCombo(true);
}

void MainWindow::onSampleComboBoxChanged(int index)
{
    upadteSampleEditors();
}

void MainWindow::onMeasurementTypeChanged()
{
    if(ui->radioMeasureSecond->isChecked())
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageSecondMeasure);
    else
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageTripleMeasure);
    updateStageLabels();
}

//----------------------------------------------- END PAGE 0 INITIAL DATA ------------------------------------------


//----------------------------------------------- PAGE 1 DRYMASS ----------------------------------------------------

void MainWindow::onDryMassExecuteStepOneClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");

    radwagScaleConnector->sendZeroCommand();
    radwagScaleConnector->sendTareCommand();
    ui->step1CheckBox->setChecked(true);
}

void MainWindow::onDryMassExecuteStepTwoClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");

    radwagScaleConnector->sendImmediateWeightCommand();
    ui->step2CheckBox->setChecked(true);
}

void MainWindow::onDryMassExecuteStepThreeClicked()
{
    if(!ui->editCurrentDryMeasure->text().isEmpty())
    {
        onSaveCurrentDryMeasureButtonClicked();
        ui->step3CheckBox->setChecked(true);
    }
    else
        QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
}

void MainWindow::onGetCurrentDryMeasureButtonClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        return;
    radwagScaleConnector->sendImmediateWeightCommand();
}

void MainWindow::onSaveCurrentDryMeasureButtonClicked()
{
    QString text = ui->editCurrentDryMeasure->text().trimmed();
    if(!radwagMeasureControler->setDryMass(utils::getDouble(text)))
        QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru!");
    ui->editSavedDryMeasure->setText(text);
}

void MainWindow::onClearSavedDryMeasureButtonClicked()
{
    ui->editSavedDryMeasure->clear();
    radwagMeasureControler->setDryMass(0.0);
}

//----------------------------------------------- END PAGE 1 DRYMASS ------------------------------------------


//----------------------------------------------- PAGE 3 FINISH SECOND ----------------------------------------

void MainWindow::onFinishMeasureSecondStepTwoClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
         QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");

    radwagScaleConnector->sendImmediateWeightCommand();
    ui->step2FinishMeasureSecondCheckBox->setChecked(true);
}

void MainWindow::onFinishMeasureSecondStepTreeClicked()
{
    if(!ui->editCurrentValueFinishSecond->text().isEmpty())
    {
        onSaveCurrentFinishMeasureSecondButtonClicked();
        ui->step3FinishMeasureSecondCheckBox->setChecked(true);
    }
    else
        QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
}

void MainWindow::onFinishMeasureTripleStepTwoClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
    {
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
        return;
    }

    radwagScaleConnector->sendImmediateWeightCommand();
    ui->step2FinishTriple->setChecked(true);
}

void MainWindow::onFinishMeasureTripleExecuteStepTreeClicked()
{
    if(!ui->editCurrentValueFinishMeasurementTriple->text().isEmpty())
    {
        onSaveTripleCurrentFinishMeasureButtonClicked();
        ui->step3FinishTriple->setChecked(true);
    }
    else
        QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
}

void MainWindow::onSaturatedMassTripleStepFourClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
    {
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
        return;
    }

    radwagScaleConnector->sendImmediateWeightCommand();
    ui->step4CheckBoxSaturatedTriple->setChecked(true);
}

void MainWindow::onSaturatedMassTripleStepFiveClicked()
{
    if(!ui->editCurrentMeasureSaturated->text().isEmpty())
    {
        onSaveTripleCurrentSaturatedMeasureButtonClicked();
        ui->step5CheckBoxSaturatedTriple->setChecked(true);
    }
    else
        QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
}

void MainWindow::onGetCurrentFinishMeasureSecondButtonClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
    radwagScaleConnector->sendImmediateWeightCommand();
}

void MainWindow::onSaveCurrentFinishMeasureSecondButtonClicked()
{
    QString text = ui->editCurrentValueFinishSecond->text().trimmed();
    if(!radwagMeasureControler->setMassInFluid(utils::getDouble(text)))
        QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru! FinishSecond");
    ui->editSavedValueFinishMeasureSecond->setText(text);
    ui->editLiquidMeasureFinishMeasureSecond->setText(text + " g");
}

void MainWindow::onClearSavedFinishMeasureSecondButtonClicked()
{
    ui->editSavedValueFinishMeasureSecond->clear();
    ui->editLiquidMeasureFinishMeasureSecond->clear();
    radwagMeasureControler->setMassInFluid(0.0);
}

void MainWindow::onGetTripleCurrentFinishMeasureButtonClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
    radwagScaleConnector->sendImmediateWeightCommand();
}

void MainWindow::onSaveTripleCurrentFinishMeasureButtonClicked()
{
    QString text = ui->editCurrentValueFinishMeasurementTriple->text().trimmed();
    if(!radwagMeasureControler->setMassInFluid(utils::getDouble(text)))
        QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru! FinishTriple");
    ui->editSavedValueFinishMeasurementTriple->setText(text);
    ui->editLiquidMeasureFinishMeasurementTriple->setText(text + " g");
}

void MainWindow::onClearTripleSavedFinishMeasureButtonClicked()
{
    ui->editSavedValueFinishMeasurementTriple->clear();
    ui->editLiquidMeasureFinishMeasurementTriple->clear();
    radwagMeasureControler->setMassInFluid(0.0);
}

void MainWindow::onGetTripleCurrentSaturatedMeasureButtonClicked()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
        QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
    radwagScaleConnector->sendImmediateWeightCommand();
}

void MainWindow::onSaveTripleCurrentSaturatedMeasureButtonClicked()
{
    QString text = ui->editCurrentMeasureSaturated->text().trimmed();
    if(!radwagMeasureControler->setSaturatedMass(utils::getDouble(text)))
        QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru! SaturatedTriple");
    ui->editSavedMeasureSaturated->setText(text);
    ui->editAirSaturatedTriple->setText(text + " g");
}

void MainWindow::onClearTripleSavedSaturatedMeasureButtonClicked()
{
    ui->editSavedMeasureSaturated->clear();
    ui->editAirSaturatedTriple->clear();
    radwagMeasureControler->setSaturatedMass(0.0);
}

//----------------------------------------------- END PAGE 3 FINISH SECOND -------------------------------------


//----------------------------------------------- PAGE 4 SUMMARY SECOND ----------------------------------------

void MainWindow::onNewMeasureButtonClicked()
{
    radwagMeasureControler->beginNewMeasure();
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(radwagMeasureControler->getStage()));
    ui->groupBoxAdditionalSettings->setVisible(true);
    goToNextMeasureStage();
}

void MainWindow::onReplyMeasureButtonClicked()
{
    if(!radwagMeasureControler->replyActiveMeasure())
        return;
    fillInitialDataLabels();
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(radwagMeasureControler->getNextStage()));
    ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
    radwagMeasureControler->setStage(radwagMeasureControler->getNextStage());
}

void MainWindow::onSaveSecondMeasureButtonClicked()
{
    measurementManager->saveMeasurements();
}

//----------------------------------------------- END PAGE 4 SUMMARY SECOND -------------------------------------

void MainWindow::navigateToToolBoxPage(QWidget* page)
{
    if(page && ui->stackedWidget->indexOf(page) != -1)
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::goToPreviousMeasureStage()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();

    auto measure = radwagMeasureControler->getActiveMeasure();
    bool isTripleMeasurement = measure->isThreeType();

    MeasurementStages::Stage prevStage = radwagMeasureControler->getPrevStage(currentStage);
    switch(currentStage)
    {
        case MeasurementStages::Stage::StartMeasure:
            break;

        case MeasurementStages::Stage::InitialData:
            ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageStartMeasure);
            break;

        case MeasurementStages::Stage::DryMeasure:
            ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
            setEnableInitialDataPage(radwagMeasureControler->getStage() == prevStage);
            ui->groupBoxAdditionalSettings->setVisible(true);
            break;

        case MeasurementStages::Stage::PrepareSecond:
        case MeasurementStages::Stage::PrepareTriple:
            ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
            setEnableDryMeasurePage(radwagMeasureControler->getStage() == prevStage);
            break;

        case MeasurementStages::Stage::FinishSecond:
            ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
            setEnablePrepareMeasureSecondPage(radwagMeasureControler->getStage() == prevStage);
            break;

        case MeasurementStages::Stage::SaturationMass:
            ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
            setEnablePrepareSaturationPage(radwagMeasureControler->getStage() == prevStage);
            break;

        case MeasurementStages::Stage::FinishTriple:
            ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
            setEnableSaturationTrilpePage(radwagMeasureControler->getStage() == prevStage);
            break;

        default:
            return;
    }

    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
    updateStageLabels();
}

void MainWindow::goToNextMeasureStage()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    auto measure = radwagMeasureControler->getActiveMeasure();
    bool isTripleMeasurement = measure->isThreeType();
    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();

    if(!validateUIDataForCurrentStage(currentStage))
        return;

    MeasurementStages::Stage nextStage = radwagMeasureControler->getNextStage(currentStage);

    switch(currentStage)
    {
        case MeasurementStages::Stage::StartMeasure:
        {
            radwagMeasureControler->setStage(nextStage);
            setEnableInitialDataPage(radwagMeasureControler->getStage() == nextStage);
            ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
            break;
        }

        case MeasurementStages::Stage::InitialData:
        {
            if(radwagMeasureControler->setStage(nextStage))
                emit setMeasureInitialData();

            ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
            ui->groupBoxAdditionalSettings->setVisible(false);
            setEnableDryMeasurePage(radwagMeasureControler->getStage() == nextStage);
            break;
        }

        case MeasurementStages::Stage::DryMeasure:
        {
            bool setStageResult = radwagMeasureControler->setStage(nextStage);

            if(isTripleMeasurement)
            {
                if(setStageResult)
                    fillPrepareSaturationDataLabels();

                setEnablePrepareSaturationPage(radwagMeasureControler->getStage() == nextStage);
                ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
            }
            else
            {
                if(setStageResult)
                    fillPrepareDataLabels();

                setEnablePrepareMeasureSecondPage(radwagMeasureControler->getStage() == nextStage);
                ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
            }
            break;
        }

        case MeasurementStages::Stage::PrepareSecond:
        {
            if(radwagMeasureControler->setStage(nextStage))
                fillFinishMeasureSecondLabels();

            setEnableFinishMeasureSecondPage(radwagMeasureControler->getStage() == nextStage);
            ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementSecond);
            break;
        }

        case MeasurementStages::Stage::PrepareTriple:
        {
            if(radwagMeasureControler->setStage(nextStage))
                fillFinishMeasureTripleLabels();

            setEnableSaturationTrilpePage(radwagMeasureControler->getStage() == nextStage);
            ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
            break;
        }

        case MeasurementStages::Stage::SaturationMass:
        {
            if(radwagMeasureControler->setStage(nextStage))
                fillAirSaturatedTripleLabels();

            ui->measureDensityStage->setCurrentWidget(ui->pageFinishSaturatedMassTriple);
            break;
        }

        case MeasurementStages::Stage::FinishSecond:
        {
            if(radwagMeasureControler->setStage(nextStage))
            {
                fillMeasureSecondLabelsSummary();
                radwagMeasureControler->setMeasureStatus(MeasurementStatus::Completed);
                radwagMeasureControler->endMeasure();
                clearSecondMeasurePages();
            }

            ui->measureDensityStage->setCurrentWidget(ui->pageSummarySecond);
            break;
        }

        case MeasurementStages::Stage::FinishTriple:
        {
            if(radwagMeasureControler->setStage(MeasurementStages::Stage::Summary))
            {
                fillMeasureTripleLabelsSummary();
                radwagMeasureControler->setMeasureStatus(MeasurementStatus::Completed);
                radwagMeasureControler->endMeasure();
                clearTripleMeasurePages();
            }

            ui->measureDensityStage->setCurrentWidget(ui->pageSummaryTriple);
            break;
         }

        default:
            return;
    }

    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(nextStage));
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

void MainWindow::onBeginNewMeasure()
{
    radwagMeasureControler->beginNewMeasure();
}

void MainWindow::onSetInitialData()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    MeasurementType type = ui->radioMeasureSecond->isChecked() ? MeasurementType::TwoStage : MeasurementType::ThreeStage;
    auto sample = sampleManager->getSample(ui->comboBoxSampleSelection->currentData().toString());
    auto fluid = fluidManager->getFluid(ui->comboBoxFluid->currentText());
    QString author = ui->editAuthor->text().trimmed();
    radwagMeasureControler->setInitialData(type, sample, fluid, author);
}

void MainWindow::onRadwagMeasueReady(const RadwagMeasure &data)
{
    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
    if(currentStage == MeasurementStages::Stage::DryMeasure)
        ui->editCurrentDryMeasure->setText(QString::number(data.getValue(), 'f', 3) );
    else if(currentStage == MeasurementStages::Stage::FinishSecond)
        ui->editCurrentValueFinishSecond->setText(QString::number(data.getValue(), 'f', 3));
    else if(currentStage == MeasurementStages::Stage::SaturationMass)
        ui->editCurrentValueFinishMeasurementTriple->setText(QString::number(data.getValue(), 'f', 3));
    else if(currentStage == MeasurementStages::Stage::FinishTriple)
        ui->editCurrentMeasureSaturated->setText(QString::number(data.getValue(), 'f', 3));
}

void MainWindow::initControls()
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->labelDeviceNameStatusBar);
    ui->statusbar->addPermanentWidget(ui->labelConnectionStatusStatusBar);
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
    updateConnectonLabelsStatusBar(false);
    updateSaveCurrentDryMeasureButtonState();
}

void MainWindow::onConnectResult(bool connected)
{
    updateStatusConnectionLabel(connected);
    if(connected)
        radwagScaleConnector->startContinuousTransmissionCurrentUnit();
}

void MainWindow::onStartMeasureButtonClicked()
{
    if(!checkDeviceConnectionWithMessage())
        return;

    if(!ui->checkBoxStep1PrepareWorkstation->isChecked() ||
       !ui->checkBoxStep2PrepareWorkstation->isChecked() ||
       !ui->checkBoxStep3PrepareWorkstation->isChecked() ||
       !ui->checkBoxStep4PrepareWorkstation->isChecked() ||
       !ui->checkBoxStep5PrepareWorkstation->isChecked())
    {
        QMessageBox::warning(this, "Niepełne przygotowanie", "Przed rozpoczęciem pomiaru wykonaj wszystkie kroki przygotowawcze.");
        return;
    }

    if(!radwagMeasureControler->hasActiveMeasurement())
        radwagMeasureControler->beginNewMeasure();

    ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageMeasureProcess);
    goToNextMeasureStage();
}

void MainWindow::onShowHydroSetSchemeButtonClicked()
{
    QDialog *schemeDialog = new QDialog(this);
    schemeDialog->setWindowTitle("Schemat zestawu do wyznaczania gęstości ciał stałych");

    QVBoxLayout *mainLayout = new QVBoxLayout(schemeDialog);

    QLabel *imageLabel = new QLabel(schemeDialog);
    QPixmap schemeImage(":/schema_img/schema_density_solids.png");
    if (schemeImage.isNull())
    {
        imageLabel->setText("Nie można załadować obrazka!");
    }
    else
    {
        QLabel *noteLabel = new QLabel(schemeDialog);
        noteLabel->setText("Elementy oznaczone <font color='yellow'>kolorem żółtym </font> są niezbędne do przeprowadzenia właściwego pomiaru.");
        noteLabel->setAlignment(Qt::AlignCenter);

        QPalette pal = noteLabel->palette();
        pal.setColor(QPalette::Window, QColor(240, 240, 240));
        noteLabel->setAutoFillBackground(false);
        noteLabel->setPalette(pal);
        noteLabel->setFrameShape(QFrame::Box);
        noteLabel->setFrameShadow(QFrame::Sunken);
        noteLabel->setLineWidth(1);
        noteLabel->setMargin(8);

        mainLayout->addWidget(noteLabel);

        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int maxWidth = screenGeometry.width() * 0.8;  // 80% szerokości ekranu
        int maxHeight = screenGeometry.height() * 0.8;  // 80% wysokości ekranu
        if(schemeImage.width() > maxWidth || schemeImage.height() > maxHeight)
            schemeImage = schemeImage.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(schemeImage);
    }

    mainLayout->addWidget(imageLabel);
    schemeDialog->setLayout(mainLayout);

    schemeDialog->exec();
    delete schemeDialog;
}

void MainWindow::onConfrimPrepareWorkstationButtonClicked()
{
    ui->checkBoxStep1PrepareWorkstation->setChecked(true);
    ui->checkBoxStep2PrepareWorkstation->setChecked(true);
    ui->checkBoxStep3PrepareWorkstation->setChecked(true);
    ui->checkBoxStep4PrepareWorkstation->setChecked(true);
    ui->checkBoxStep5PrepareWorkstation->setChecked(true);
}

void MainWindow::onConfrimPrepareMeasureSecondButtonClicked()
{
    ui->stepFirstPrepareMeasureSecond->setChecked(true);
    ui->stepTwoPrepareMeasureSecond->setChecked(true);
    ui->stepTreePrepareMeasureSecond->setChecked(true);
    ui->stepFourPrepareMeasureSecond->setChecked(true);
    ui->stepFivePrepareMeasureSecond->setChecked(true);
}

bool MainWindow::checkGuidePrepareMeasureSecondButton()
{
    if(!ui->stepFirstPrepareMeasureSecond->isChecked() ||
        !ui->stepTwoPrepareMeasureSecond->isChecked() ||
        !ui->stepTreePrepareMeasureSecond->isChecked() ||
        !ui->stepFourPrepareMeasureSecond->isChecked() ||
        !ui->stepFivePrepareMeasureSecond->isChecked())
    {
        QMessageBox::warning(this, "Niepełne przygotowanie", "Przed rozpoczęciem pomiaru wykonaj wszystkie kroki przygotowawcze (część druga).");
        return false;
    }
    return true;
}

void MainWindow::onConfirmSampleSaturationPreparationClicked()
{
    ui->step1CheckBoxPrepareSaturation->setChecked(true);
    ui->step2CheckBoxPrepareSaturation->setChecked(true);
    ui->step3CheckBoxPrepareSaturation->setChecked(true);
    ui->step4CheckBoxPrepareSaturation->setChecked(true);
    ui->step5CheckBoxPrepareSaturation->setChecked(true);
    ui->step6CheckBoxPrepareSaturation->setChecked(true);
}

bool MainWindow::checkGuideSampleSaturationPreparation()
{
    if(!ui->step1CheckBoxPrepareSaturation->isChecked() ||
        !ui->step2CheckBoxPrepareSaturation->isChecked() ||
        !ui->step3CheckBoxPrepareSaturation->isChecked() ||
        !ui->step4CheckBoxPrepareSaturation->isChecked() ||
        !ui->step5CheckBoxPrepareSaturation->isChecked() ||
        !ui->step6CheckBoxPrepareSaturation->isChecked())
    {
        QMessageBox::warning(this, "Niepełne przygotowanie", "Przed kontynuacją pomiaru wykonaj wszystkie wymagane kroki.");
        return false;
    }
    return true;
}

void MainWindow::connectButtons()
{
    connectMainNavButtons();
    connectDevicesSettingsButtons();
    connectNavMeasurementButtons();
    connectPrepareWorksationPageButtons();
    connectInitialDataPageButtons();
    connectDryMassPageButtons();
    connectPrepareMeasureSecondPageButtons();
    connectPrepareSaturationButton();
    connectFinishSecondPageButtons();
    connectFinishTriplePageButtons();
    connectSaturatedTriplePageButtons();
    connectSummaryMeasureSecondPageButtons();
    connectSummaryMeasureTriplePageButtons();
    connectCatalogsButtons();
}

void MainWindow::connectMainNavButtons()
{
    connect(ui->actionSettings, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->settingsPage);
    });

    connect(ui->actionMeasureDensity, &QAction::triggered, this, [this]() {
        navigateToToolBoxPage(ui->measureDensityPage);
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onMainPageChanged);
}

void MainWindow::connectNavMeasurementButtons()
{
    connect(ui->buttonPrevData, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonNextData, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);

    connect(ui->buttonNextDryMass, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevDryMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonNextPreparation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevPreparation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->buttonNextPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonNextSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevSaturatedMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
}

void MainWindow::connectPrepareWorksationPageButtons()
{
    connect(ui->buttonShowHydroSetScheme, &QPushButton::clicked, this, &MainWindow::onShowHydroSetSchemeButtonClicked);
    connect(ui->buttonConfrimPrepareWorkstation, &QPushButton::clicked, this, &MainWindow::onConfrimPrepareWorkstationButtonClicked);
    connect(ui->buttonStartMeasure, &QPushButton::clicked, this, &MainWindow::onStartMeasureButtonClicked);
}

void MainWindow::connectInitialDataPageButtons()
{
    connect(ui->comboBoxSelectDevice, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
    connect(ui->buttonConnectDevice, &QPushButton::clicked, this, &MainWindow::onConnectDeviceClicked);
    connect(ui->buttonDisconnectDevice, &QPushButton::clicked, this, &MainWindow::onDisconnectDeviceClicked);

    connect(ui->radioMeasureSecond, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);
    connect(ui->radioMeasureTriple, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);
    connect(ui->comboBoxSampleSelection, &QComboBox::currentIndexChanged, this, &MainWindow::onSampleComboBoxChanged);

    connect(this, &MainWindow::setMeasureInitialData, this, &MainWindow::onSetInitialData);
}

void MainWindow::connectPrepareMeasureSecondPageButtons()
{
    connect(ui->buttonConfrimPrepareMeasureSecond, &QPushButton::clicked, this, &MainWindow::onConfrimPrepareMeasureSecondButtonClicked);
}

void MainWindow::connectPrepareSaturationButton()
{
    connect(ui->buttonConfirmPrepareSaturation, &QPushButton::clicked, this, &MainWindow::onConfirmSampleSaturationPreparationClicked);
    connect(ui->comboBoxSaturationMethodPrepareMeasureTriple, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateSaturationMethodPrepareTriple);
    connect(ui->comboBoxTempFluidPrepareSaturationTrilpe, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePrepareSaturationFluidDensityLabel);
    connect(ui->spinSaturationTimePrepareMeasureTriple, &QSpinBox::valueChanged, this, &MainWindow::onSpinSaturationTimeChanged);
}

void MainWindow::connectDryMassPageButtons()
{
    connect(ui->buttonDryMassExecuteStepOne, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepOneClicked);
    connect(ui->buttonDryMassExecuteStepTwo, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepTwoClicked);
    connect(ui->buttonDryMassExecuteStepThree, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepThreeClicked);

    connect(ui->buttonGetCurrentDryMeasure, &QPushButton::clicked, this, &MainWindow::onGetCurrentDryMeasureButtonClicked);
    connect(ui->buttonSaveCurrentDryMeasure, &QPushButton::clicked, this, &MainWindow::onSaveCurrentDryMeasureButtonClicked);
    connect(ui->buttonClearEditSavedDryMeasure, &QPushButton::clicked, this, &MainWindow::onClearSavedDryMeasureButtonClicked);

    connect(ui->editCurrentDryMeasure, &QLineEdit::textChanged, this, &MainWindow::updateSaveCurrentDryMeasureButtonState);
}

void MainWindow::connectFinishSecondPageButtons()
{
    connect(ui->buttonFinishMeasureSecondExecuteStepTwo, &QPushButton::clicked, this, &MainWindow::onFinishMeasureSecondStepTwoClicked);
    connect(ui->buttonFinishMeasureSecondExecuteStepTree, &QPushButton::clicked, this, &MainWindow::onFinishMeasureSecondStepTreeClicked);

    connect(ui->buttonGetCurrentValueFinishMeasureSecond, &QPushButton::clicked, this, &MainWindow::onGetCurrentFinishMeasureSecondButtonClicked);
    connect(ui->buttonSaveCurrentFinishSecond, &QPushButton::clicked, this, &MainWindow::onSaveCurrentFinishMeasureSecondButtonClicked);
    connect(ui->buttonClearSavedValueFinishMeasureSecond, &QPushButton::clicked, this, &MainWindow::onClearSavedFinishMeasureSecondButtonClicked);

    connect(ui->editCurrentValueFinishSecond, &QLineEdit::textChanged, this, &MainWindow::updateSaveFinishSecondButtonState);
}

void MainWindow::connectFinishTriplePageButtons()
{
    connect(ui->buttonFinishMeasureTripleExecuteStepTwo, &QPushButton::clicked, this, &MainWindow::onFinishMeasureTripleStepTwoClicked);
    connect(ui->buttonFinishMeasureTripleExecuteStepThree, &QPushButton::clicked, this, &MainWindow::onFinishMeasureTripleExecuteStepTreeClicked);

    connect(ui->buttonGetCurrentMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::onGetTripleCurrentFinishMeasureButtonClicked);
    connect(ui->buttonSaveMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::onSaveTripleCurrentFinishMeasureButtonClicked);
    connect(ui->buttonClearSavedMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::onClearTripleSavedFinishMeasureButtonClicked);

    connect(ui->editCurrentValueFinishMeasurementTriple, &QLineEdit::textChanged, this, &MainWindow::updateSaveFinishSecondButtonState);
}

void MainWindow::connectSaturatedTriplePageButtons()
{
    connect(ui->buttonSaturatedMassExecuteStepFour, &QPushButton::clicked, this, &MainWindow::onSaturatedMassTripleStepFourClicked);
    connect(ui->buttonSaturatedMassExecuteStepFive, &QPushButton::clicked, this, &MainWindow::onSaturatedMassTripleStepFiveClicked);

    connect(ui->buttonGetMeasureSaturated, &QPushButton::clicked, this, &MainWindow::onGetTripleCurrentSaturatedMeasureButtonClicked);
    connect(ui->buttonSaveCurrentMeasureSaturated, &QPushButton::clicked, this, &MainWindow::onSaveTripleCurrentSaturatedMeasureButtonClicked);
    connect(ui->buttonClearSavedMeasureSaturated, &QPushButton::clicked, this, &MainWindow::onClearTripleSavedSaturatedMeasureButtonClicked);

    connect(ui->editCurrentMeasureSaturated, &QLineEdit::textChanged, this, &MainWindow::updateSaveSaturatedTripleButtonState);
}

void MainWindow::connectSummaryMeasureSecondPageButtons()
{
    connect(ui->buttonNewSecondMeasure, &QPushButton::clicked, this, &MainWindow::onNewMeasureButtonClicked);
    connect(ui->buttonReplySecondMeasure, &QPushButton::clicked, this, &MainWindow::onReplyMeasureButtonClicked);
    connect(ui->buttonSaveSecondMeasureToLibrary, &QPushButton::clicked, this, &MainWindow::onSaveSecondMeasureButtonClicked);
}

void MainWindow::connectSummaryMeasureTriplePageButtons()
{
    connect(ui->buttonNewTripleMeasure, &QPushButton::clicked, this, &MainWindow::onNewMeasureButtonClicked);
    connect(ui->buttonReplyTripleMeasure, &QPushButton::clicked, this, &MainWindow::onReplyMeasureButtonClicked);
}

void MainWindow::connectCatalogsButtons()
{
    connect(ui->buttonTableFluids, &QPushButton::clicked, this, &MainWindow::buttonTableFluidsOnClicked);
    connect(ui->buttonSamples, &QPushButton::clicked, this, &MainWindow::buttonSamplesOnClicked);
}

void MainWindow::connectDevicesSettingsButtons()
{
    connect(ui->addDeviceButton, &QPushButton::clicked, this, &MainWindow::onAddDeviceButtonClicked);
    connect(ui->editDeviceButton, &QPushButton::clicked, this, &MainWindow::onEditDeviceButtonClicked);
    connect(ui->deleteDeviceButton, &QPushButton::clicked, this, &MainWindow::onRemoveDeviceButtonClicked);
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

    QPalette measurePalette;
    measurePalette.setColor(QPalette::WindowText, MEASURE_LABEL_COLOR);

    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
    MeasurementStages::Stage radwagStage = radwagMeasureControler->getStage();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    if(isTripleMeasurement)
    {
        QList<QLabel*> tripleLabels =
            {
                ui->labelStageDataTriple,
                ui->labelStageDryMassTriple,
                ui->labelStagePrepareSaturation,
                ui->labelStageSaturationMass,
                ui->labelStageSaturatedMass,
                ui->labelStageSummaryTriple
            };

        for (QLabel* label : tripleLabels)
        {
            label->setFont(normalFont);
            label->setPalette(normalPalette);
        }

        switch (currentStage)
        {
            case MeasurementStages::Stage::InitialData:
                ui->labelStageDataTriple->setFont(boldFont);
                ui->labelStageDataTriple->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::DryMeasure:
                ui->labelStageDryMassTriple->setFont(boldFont);
                ui->labelStageDryMassTriple->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::PrepareTriple:
                ui->labelStagePrepareSaturation->setFont(boldFont);
                ui->labelStagePrepareSaturation->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::SaturationMass:
                ui->labelStageSaturationMass->setFont(boldFont);
                ui->labelStageSaturationMass->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::FinishTriple:
                ui->labelStageSaturatedMass->setFont(boldFont);
                ui->labelStageSaturatedMass->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::Summary:
                ui->labelStageSummaryTriple->setFont(boldFont);
                ui->labelStageSummaryTriple->setPalette(activePalette);
                break;
            default:
                break;
        }

        if(static_cast<int>(radwagStage) == static_cast<int>(currentStage))
            return;

        switch(radwagStage)
        {
            case MeasurementStages::Stage::InitialData:
                ui->labelStageDataTriple->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::DryMeasure:
                ui->labelStageDryMassTriple->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::PrepareTriple:
                ui->labelStagePrepareSaturation->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::SaturationMass:
                ui->labelStageSaturationMass->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::FinishTriple:
                ui->labelStageSaturatedMass->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::Summary:
                ui->labelStageSummaryTriple->setPalette(measurePalette);
                break;
            default:
                break;
        }
    }
    else
    {
        QList<QLabel*> secondLabels =
            {
                ui->labelStageData,
                ui->labelStageDryMass,
                ui->labelStagePreparation,
                ui->labelStageFluidMass,
                ui->labelStageSummary
            };

        for(QLabel* label : secondLabels)
        {
            label->setFont(normalFont);
            label->setPalette(normalPalette);
        }

        switch (currentStage)
        {
            case MeasurementStages::Stage::InitialData:
                ui->labelStageData->setFont(boldFont);
                ui->labelStageData->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::DryMeasure:
                ui->labelStageDryMass->setFont(boldFont);
                ui->labelStageDryMass->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::PrepareSecond:
            case MeasurementStages::Stage::SaturationMass:
                ui->labelStagePreparation->setFont(boldFont);
                ui->labelStagePreparation->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::FinishSecond:
                ui->labelStageFluidMass->setFont(boldFont);
                ui->labelStageFluidMass->setPalette(activePalette);
                break;
            case MeasurementStages::Stage::Summary:
                ui->labelStageSummary->setFont(boldFont);
                ui->labelStageSummary->setPalette(activePalette);
                break;
            default:
                break;
        }

        if(static_cast<int>(radwagStage) == static_cast<int>(currentStage))
            return;

        switch(radwagStage)
        {
            case MeasurementStages::Stage::InitialData:
                ui->labelStageData->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::DryMeasure:
                ui->labelStageDryMass->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::PrepareSecond:
            case MeasurementStages::Stage::SaturationMass:
                ui->labelStagePreparation->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::FinishSecond:
                ui->labelStageFluidMass->setPalette(measurePalette);
                break;
            case MeasurementStages::Stage::Summary:
                ui->labelStageSummary->setPalette(measurePalette);
                break;
            default:
                break;
        }
    }
}

void MainWindow::setProperty()
{
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::StartMeasure));
    ui->scrollAreaInitialData->setBackgroundRole(QPalette::Base);
    ui->scrollArea->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishSecond->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishTriple->setBackgroundRole(QPalette::Base);
    ui->scrollAreaLibrary->setBackgroundRole(QPalette::Base);
    ui->scrollAreaLibrary->setBackgroundRole(QPalette::Base);
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}

//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------

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
    updateConnectonLabelsStatusBar(connectionStatus);
}

void MainWindow::updateConnectonLabelsStatusBar(bool connectionStatus)
{
    if(connectionStatus)
    {
        ui->labelDeviceNameStatusBar->setText("Urządzenie: " + ui->comboBoxSelectDevice->currentText());
        ui->labelConnectionStatusStatusBar->setText("Status: <font color='green'><b>Połączono</b></font>");
        ui->comboBoxSelectDevice->setEnabled(false);
    }
    else
    {
        QString deviceName = ui->comboBoxSelectDevice->currentText();
        ui->labelDeviceNameStatusBar->setText("Urządzenie: " + (deviceName.isEmpty() ? "Nie wybrano" : ui->comboBoxSelectDevice->currentText()));
        ui->labelConnectionStatusStatusBar->setText("Status: <font color='red'><b>Brak połączenia</b></font>");
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
    ui->editMaterialDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 4) +  " g/cm³");
    ui->editSampleDescription->setPlainText(sample.getDescription());
}

void MainWindow::clearSecondMeasurePages()
{
    clearInitialDataPage();
    clearDryMeasurePage();
    clearPrepareMeasureSecondPage();
    clearFinishMeasureSecondPage();
}

void MainWindow::clearTripleMeasurePages()
{
    clearInitialDataPage();
    clearDryMeasurePage();
    clearPrepareMeasureTriplePage();
    clearFinishMeasureTriplePage();
    clearAirSaturatedTriplePage();
}

void MainWindow::clearSampleEditors()
{
    ui->editSampleId->clear();
    ui->editSampleName->clear();
    ui->editMaterial->clear();
    ui->editMaterialDensity->clear();
    ui->editSampleDescription->clear();
}

void MainWindow::clearInitialDataPage()
{
    ui->comboBoxSampleSelection->setCurrentIndex(-1);
    ui->editSampleId->clear();
    ui->editSampleName->clear();
    ui->editMaterial->clear();
    ui->editMaterialDensity->clear();
    ui->editAuthor->clear();
    ui->editSampleDescription->clear();
    ui->comboBoxFluid->setCurrentIndex(-1);
}

void MainWindow::clearDryMeasurePage()
{
    ui->step1CheckBox->setChecked(false);
    ui->step2CheckBox->setChecked(false);
    ui->step3CheckBox->setChecked(false);

    ui->editCurrentDryMeasure->clear();
    ui->editSavedDryMeasure->clear();
}

void MainWindow::clearPrepareMeasureSecondPage()
{
    ui->stepFirstPrepareMeasureSecond->setChecked(false);
    ui->stepTwoPrepareMeasureSecond->setChecked(false);
    ui->stepTreePrepareMeasureSecond->setChecked(false);
    ui->stepFourPrepareMeasureSecond->setChecked(false);
    ui->stepFivePrepareMeasureSecond->setChecked(false);
    ui->comboBoxTempFluidPrepareMeasureSecond->clear();
    ui->editSampleIdValuePrepareMeasureSecond->clear();
    ui->editLiquidTypePrepareMeasureSecond->clear();
    ui->editSampleMaterialValuePrepareMeasureSecond->clear();
    ui->labelLiquidDensityValuePrepareMeasureSecond->clear();
    ui->labelAirWeightValuePrepareMeasureSecond->clear();
}

void MainWindow::clearFinishMeasureSecondPage()
{
    ui->step3CheckBox_6->setChecked(false);
    ui->step2FinishMeasureSecondCheckBox->setChecked(false);
    ui->step3FinishMeasureSecondCheckBox->setChecked(false);

    ui->editCurrentValueFinishSecond->clear();
    ui->editSavedValueFinishMeasureSecond->clear();
    ui->editDryMeasureFinishMeasureSecond->clear();
    ui->editLiquidMeasureFinishMeasureSecond->clear();
    ui->editSampleIdValueFinishMeasureSecond->clear();
    ui->editSampleMaterialValueFinishMeasureSecond->clear();
    ui->editLiquidFinishMeasureSecond->clear();
    ui->editDensityLiquidFinishMeasureSecond->clear();
}

void MainWindow::clearPrepareMeasureTriplePage()
{
    ui->step1CheckBoxPrepareSaturation->setChecked(false);
    ui->step2CheckBoxPrepareSaturation->setChecked(false);
    ui->step3CheckBoxPrepareSaturation->setChecked(false);
    ui->step4CheckBoxPrepareSaturation->setChecked(false);
    ui->step5CheckBoxPrepareSaturation->setChecked(false);
    ui->step6CheckBoxPrepareSaturation->setChecked(false);

    ui->comboBoxTempFluidPrepareSaturationTrilpe->clear();
    ui->comboBoxSaturationMethodPrepareMeasureTriple->clear();
    ui->spinSaturationTimePrepareMeasureTriple->setValue(0);
    ui->editStartSaturationDateTimePrepareMeasureTriple->setDateTime(QDateTime::currentDateTime());

    ui->ediSampleIdValuePrepareMeasureTriple->clear();
    ui->editSampleMaterialValuePrepareMeasureTriple->clear();
    ui->editLiquidTypePrepareSaturationTriple->clear();
    ui->labelFluidDensityPrepareSaturationTrilpe->clear();
    ui->editAirWeightValuePrepareMeasureTriple->clear();
}

void MainWindow::clearFinishMeasureTriplePage()
{
    ui->step1FinishTriple->setChecked(false);
    ui->step2FinishTriple->setChecked(false);
    ui->step3FinishTriple->setChecked(false);

    ui->editCurrentValueFinishMeasurementTriple->clear();
    ui->editSavedValueFinishMeasurementTriple->clear();
    ui->editDryMassSaturationTriple->clear();
    ui->editLiquidMeasureFinishMeasurementTriple->clear();

    ui->editSampleIdValueSaturationTriple->clear();
    ui->editMaterialNameSaturationTriple->clear();
    ui->editFluidNameSaturationTriple->clear();
    ui->editSaturationMethodSaturationTriple->clear();
    ui->editFluidDensitySaturationTriple->clear();
}

void MainWindow::clearAirSaturatedTriplePage()
{
    ui->step1CheckBox_2->setChecked(false);
    ui->step2CheckBox_2->setChecked(false);
    ui->step3CheckBox_2->setChecked(false);
    ui->step4CheckBoxSaturatedTriple->setChecked(false);
    ui->step5CheckBoxSaturatedTriple->setChecked(false);

    ui->editCurrentMeasureSaturated->clear();
    ui->editSavedMeasureSaturated->clear();
    ui->editDryAirSaturatedTriple->clear();
    ui->editLiquidAirSaturatedTriple->clear();
    ui->editAirSaturatedTriple->clear();

    ui->editSampleIdAirSaturatedTriple->clear();
    ui->editMaterialAirSaturatedTriple->clear();
    ui->editFluidAirSaturatedTriple->clear();
    ui->editSaturationAirSaturatedTriple->clear();
    ui->editDensityAirSaturatedTriple->clear();
}

void MainWindow::setEnableInitialDataPage(bool enabled)
{
    ui->radioMeasureSecond->setEnabled(enabled);
    ui->radioMeasureTriple->setEnabled(enabled);
    ui->comboBoxSampleSelection->setEnabled(enabled);
    ui->editAuthor->setEnabled(enabled);
    ui->comboBoxFluid->setEnabled(enabled);
    ui->buttonSamples->setEnabled(enabled);
    ui->buttonTableFluids->setEnabled(enabled);
}

void MainWindow::setEnableDryMeasurePage(bool enabled)
{
    ui->step1CheckBox->setEnabled(enabled);
    ui->step2CheckBox->setEnabled(enabled);
    ui->step3CheckBox->setEnabled(enabled);
    ui->buttonDryMassExecuteStepOne->setEnabled(enabled);
    ui->buttonDryMassExecuteStepTwo->setEnabled(enabled);
    ui->buttonDryMassExecuteStepThree->setEnabled(enabled);

    ui->buttonGetCurrentDryMeasure->setEnabled(enabled);
    ui->buttonSaveCurrentDryMeasure->setEnabled(enabled);
    ui->buttonClearEditSavedDryMeasure->setEnabled(enabled);
}

void MainWindow::setEnablePrepareMeasureSecondPage(bool enabled)
{
    ui->stepFirstPrepareMeasureSecond->setEnabled(enabled);
    ui->stepTwoPrepareMeasureSecond->setEnabled(enabled);
    ui->stepTreePrepareMeasureSecond->setEnabled(enabled);
    ui->stepFourPrepareMeasureSecond->setEnabled(enabled);
    ui->stepFivePrepareMeasureSecond->setEnabled(enabled);
    ui->buttonConfrimPrepareMeasureSecond->setEnabled(enabled);
    ui->comboBoxTempFluidPrepareMeasureSecond->setEnabled(enabled);
}

void MainWindow::setEnableFinishMeasureSecondPage(bool enabled)
{
    ui->step3CheckBox_6->setEnabled(enabled);
    ui->step2FinishMeasureSecondCheckBox->setEnabled(enabled);
    ui->step3FinishMeasureSecondCheckBox->setEnabled(enabled);

    ui->buttonFinishMeasureSecondExecuteStepTwo->setEnabled(enabled);
    ui->buttonFinishMeasureSecondExecuteStepTree->setEnabled(enabled);
    ui->buttonGetCurrentValueFinishMeasureSecond->setEnabled(enabled);
    ui->buttonSaveCurrentFinishSecond->setEnabled(enabled);
    ui->buttonClearSavedValueFinishMeasureSecond->setEnabled(enabled);
}

void MainWindow::setEnablePrepareSaturationPage(bool enabled)
{
    ui->comboBoxSaturationMethodPrepareMeasureTriple->setEnabled(enabled);
    ui->spinSaturationTimePrepareMeasureTriple->setEnabled(enabled);
    ui->editStartSaturationDateTimePrepareMeasureTriple->setEnabled(enabled);
    ui->comboBoxTempFluidPrepareSaturationTrilpe->setEnabled(enabled);

    ui->step1CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->step2CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->step3CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->step4CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->step5CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->step6CheckBoxPrepareSaturation->setEnabled(enabled);
    ui->buttonConfirmPrepareSaturation->setEnabled(enabled);
}

void MainWindow::setEnableSaturationTrilpePage(bool enabled)
{
    ui->step1FinishTriple->setEnabled(enabled);
    ui->step2FinishTriple->setEnabled(enabled);
    ui->step3FinishTriple->setEnabled(enabled);

    ui->buttonFinishMeasureTripleExecuteStepTwo->setEnabled(enabled);
    ui->buttonFinishMeasureTripleExecuteStepThree->setEnabled(enabled);

    ui->buttonGetCurrentMeasureFinishMeasurementTriple->setEnabled(enabled);
    ui->buttonSaveMeasureFinishMeasurementTriple->setEnabled(enabled);
    ui->buttonClearSavedMeasureFinishMeasurementTriple->setEnabled(enabled);
}

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------


//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------

void MainWindow::updateSaveCurrentDryMeasureButtonState()
{
    ui->buttonSaveCurrentDryMeasure->setEnabled(utils::getDouble(ui->editCurrentDryMeasure->text()) > 0.0);
}

bool MainWindow::validateUIDataForCurrentStage(MeasurementStages::Stage currentStage)
{
    switch(currentStage)
    {
        case MeasurementStages::Stage::StartMeasure:
            return true;

        case MeasurementStages::Stage::InitialData:
            return validateInitialData();

        case MeasurementStages::Stage::DryMeasure:
            return vaildateDryMeasureData();

        case MeasurementStages::Stage::PrepareSecond:
            return checkGuidePrepareMeasureSecondButton() && vaildatePrepareSecondMeasureData();

        case MeasurementStages::Stage::PrepareTriple:
            return checkGuideSampleSaturationPreparation() && vaildatePrepareTripleMeasureData();

        case MeasurementStages::Stage::SaturationMass:
            return vaildateSaturationMassMeasureData();

        case MeasurementStages::Stage::FinishSecond:
            return vaildateFinishSecondMeasureData();

        case MeasurementStages::Stage::FinishTriple:
            return vaildateFinishTripleMeasureData();

        default:
            return false;
    }
}

bool MainWindow::validateInitialData()
{
    if(!checkDeviceConnectionWithMessage())
        return false;

    if(ui->comboBoxSampleSelection->currentText().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak wybranej próbki"), "Brak wybranej próbki");
        return false;
    }

    if(ui->editAuthor->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak danych"), "Pole autor nie może być puste");
        return false;
    }

    return true;
}

bool MainWindow::vaildateDryMeasureData()
{
    checkDeviceConnectionWithMessage();

    if(ui->editSavedDryMeasure->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }

    return true;
}

bool MainWindow::vaildatePrepareSecondMeasureData()
{
    if(ui->comboBoxTempFluidPrepareMeasureSecond->currentIndex() == -1)
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz wybrać temperature cieczy");
        return false;
    }

    return true;
}

bool MainWindow::vaildateFinishSecondMeasureData()
{
    if(ui->editSavedValueFinishMeasureSecond->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }

    return true;
}

bool MainWindow::vaildatePrepareTripleMeasureData()
{
    if(ui->comboBoxTempFluidPrepareSaturationTrilpe->currentIndex() == -1)
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz wybrać temperature cieczy");
        return false;
    }

    if(ui->comboBoxSaturationMethodPrepareMeasureTriple->currentIndex() == -1)
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz wybrać metodę nasycania próbki");
        return false;
    }

    if(ui->spinSaturationTimePrepareMeasureTriple->value() == 0)
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz uzupełnić czas próbki");
        return false;
    }
    return true;
}

bool MainWindow::vaildateSaturationMassMeasureData()
{
    if(ui->editSavedValueFinishMeasurementTriple->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }

    return true;
}

bool MainWindow::vaildateFinishTripleMeasureData()
{
    if(ui->editSavedMeasureSaturated->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }

    return true;
}

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------


bool MainWindow::fillInitialDataLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return false;

    auto activeMeasure = radwagMeasureControler->getActiveMeasure();
    ui->comboBoxSampleSelection->setCurrentText(activeMeasure->getSample().getName());

    ui->editSampleId->setText(activeMeasure->getSample().getId());
    ui->editSampleName->setText(activeMeasure->getSample().getName());
    ui->editMaterial->setText(activeMeasure->getSample().getMaterialName());
    ui->editMaterialDensity->setText(QString::number(activeMeasure->getSample().getMaterialDensity()) + " g/cm³");
    ui->editSampleDescription->setPlainText(activeMeasure->getSample().getDescription());
    ui->comboBoxFluid->setCurrentText(activeMeasure->getFluidName());
    ui->editAuthor->setText(activeMeasure->getAuthor());

    return true;
}

void MainWindow::fillPrepareDataLabels()
{
    fillSampleInfoLabels();
    fillFluidInfoLabels();
}

void MainWindow::fillSampleInfoLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();

    ui->editSampleIdValuePrepareMeasureSecond->setText(currentSample.getId());
    ui->editSampleMaterialValuePrepareMeasureSecond->setText(currentSample.getMaterialName());

    double airMass = radwagMeasureControler->getDryMass();
    QString formattedAirMass = QString::number(airMass, 'f', 3) + " g";
    ui->labelAirWeightValuePrepareMeasureSecond->setText(formattedAirMass);
}

void MainWindow::fillFluidInfoLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    ui->editLiquidTypePrepareMeasureSecond->setText(currentFluid.getName());
    fillTemperatureComboBox();
    updateFluidDensityLabel();

    connect(ui->comboBoxTempFluidPrepareMeasureSecond, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateFluidDensityLabel);
}

void MainWindow::fillTemperatureComboBox()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    ui->comboBoxTempFluidPrepareMeasureSecond->clear();

    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

    for(auto it = densityTableMap.keyBegin(); it != densityTableMap.keyEnd(); it++)
        ui->comboBoxTempFluidPrepareMeasureSecond->addItem(QString::number(*it) + " °C", *it);

    ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(-1);
    ui->labelLiquidDensityValuePrepareMeasureSecond->setText(QString("0.0 g/cm³"));
}

void MainWindow::fillFinishMeasureSecondLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();

    ui->editSampleIdValueFinishMeasureSecond->setText(currentSample.getId());
    ui->editSampleMaterialValueFinishMeasureSecond->setText(currentSample.getMaterialName());
    ui->editLiquidFinishMeasureSecond->setText(currentFluid.getName());

    double airMass = radwagMeasureControler->getDryMass();
    double fluidDensity = radwagMeasureControler->getFluidDensity();

    ui->editDryMeasureFinishMeasureSecond->setText(QString::number(airMass, 'f', 3) + " g");
    ui->editDensityLiquidFinishMeasureSecond->setText(QString::number(fluidDensity, 'f', 5) + " g/cm³");
}

void MainWindow::updateFluidDensityLabel()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    int currentIndex = ui->comboBoxTempFluidPrepareMeasureSecond->currentIndex();
    if(currentIndex == -1)
        return;

    double selectedTemperature = ui->comboBoxTempFluidPrepareMeasureSecond->itemData(currentIndex).toDouble();

    Fluid currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    const QMap<double, double>& densityTableMap = currentFluid.getDensityTableMap();

    if(!densityTableMap.contains(selectedTemperature))
        return;

    double density = densityTableMap.value(selectedTemperature);
    QString formattedDensity = QString::number(density, 'f', 5) + " g/cm³";
    ui->labelLiquidDensityValuePrepareMeasureSecond->setText(formattedDensity);
    radwagMeasureControler->setFluidTemperature(selectedTemperature);
}

void MainWindow::updateSaveFinishSecondButtonState()
{
    ui->buttonSaveCurrentFinishSecond->setEnabled(utils::getDouble(ui->editCurrentValueFinishSecond->text()) > 0.0);
}

void MainWindow::updateSaveFinishTripleButtonState()
{
    ui->buttonSaveMeasureFinishMeasurementTriple->setEnabled(utils::getDouble(ui->editCurrentValueFinishMeasurementTriple->text()) > 0.0);
}

void MainWindow::updateSaveSaturatedTripleButtonState()
{
    ui->buttonSaveCurrentMeasureSaturated->setEnabled(utils::getDouble(ui->editCurrentMeasureSaturated->text()) > 0.0);
}

void MainWindow::fillFinishMeasureTripleLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();

    double airMass = radwagMeasureControler->getDryMass();
    double fluidDensity = radwagMeasureControler->getFluidDensity();

    radwagMeasureControler->setSaturationMethod(static_cast<SaturationMethod>(ui->comboBoxSaturationMethodPrepareMeasureTriple->currentData().toInt()));

    ui->editSampleIdValueSaturationTriple->setText(currentSample.getId());
    ui->editMaterialNameSaturationTriple->setText(currentSample.getMaterialName());
    ui->editFluidNameSaturationTriple->setText(currentFluid.getName());
    ui->editSaturationMethodSaturationTriple->setText(utils::getSaturationMethodName(static_cast<SaturationMethod>(radwagMeasureControler->getActiveMeasure()->getSaturationMethod())));
    ui->editFluidDensitySaturationTriple->setText(QString::number(fluidDensity, 'f', 5) + " g");
    ui->editDryMassSaturationTriple->setText(QString::number(airMass, 'f', 3) + " g");
}

void MainWindow::fillAirSaturatedTripleLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();

    ui->editSampleIdAirSaturatedTriple->setText(currentSample.getId());
    ui->editMaterialAirSaturatedTriple->setText(currentSample.getMaterialName());
    ui->editFluidAirSaturatedTriple->setText(currentFluid.getName());
    ui->editSaturationAirSaturatedTriple->setText(utils::getSaturationMethodName(static_cast<SaturationMethod>(radwagMeasureControler->getActiveMeasure()->getSaturationMethod())));

    double airMass = radwagMeasureControler->getDryMass();
    ui->editDryAirSaturatedTriple->setText(QString::number(airMass, 'f', 3) + " g");

    double fluidDensity = radwagMeasureControler->getFluidDensity();
    ui->editDensityAirSaturatedTriple->setText(QString::number(fluidDensity, 'f', 5) + " g/cm³");

    double saturationMassInFluid = radwagMeasureControler->getMassInFluid();
    ui->editLiquidAirSaturatedTriple->setText(QString::number(saturationMassInFluid, 'f', 3) + " g");
}

void MainWindow::fillMeasureTripleLabelsSummary()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    auto measurement = radwagMeasureControler->getActiveMeasure();
    auto results = radwagMeasureControler->calculateResults();
    auto sample = measurement->getSample();
    Fluid fluid = measurement->getFluid();

    ui->valueTripleMeasureID->setText(measurement->getSampleId());
    ui->valueTripleSampleName->setText(sample.getName());
    ui->valueTripleMaterialName->setText(sample.getMaterialName());
    ui->valueTripleTheoreticalDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 4) + " g/cm³");

    ui->valueTripleMeasureLiquidName->setText(fluid.getName());
    ui->valueTripleMeasureLiquidDensity->setText(QString::number(measurement->getFluidDensity(), 'f', 5) + " g/cm³");
    ui->valueTripleMeasureSaturationMethod->setText(utils::getSaturationMethodName(measurement->getSaturationMethod()));
    ui->valueTripleMeasureSaturationTime->setText(QString::number(measurement->getSaturationTime()) + " min");

    ui->valueTripleMeasureType->setText(measurement->getType() == MeasurementType::TwoStage ? "Dwustopniowy" : "Trzystopniowy");
    ui->valueTripleMeasureOperator->setText(measurement->getAuthor());
    QString dateTime = measurement->getDate().toString("dd-MM-yyyy HH:mm");
    ui->valueTripleMeasureDateTime->setText(dateTime);

    double dryMass = measurement->getSampleDryMass();
    ui->valueTripleMeasureDryMass->setText(QString::number(dryMass, 'f', 3) + " g");

    double massInFluid = measurement->getSampleInFluidMass();
    ui->valueTripleMeasureWetMass->setText(QString::number(massInFluid, 'f', 3) + " g");

    double saturatedMass = measurement->getSampleSaturatedMass();
    ui->valueTripleMeasureSaturatedMass->setText(QString::number(saturatedMass, 'f', 3) + " g");

    double apparentDensity = results.getApparentDensity();
    ui->valueTripleMeasureApparentDensity->setText(QString::number(apparentDensity, 'f', 3) + " g/cm³");

    double relativeDensity = results.getRelativeDensity();
    ui->valueTripleMeasureRelativeDensity->setText(QString::number(relativeDensity, 'f', 2) + " %");

    double apparentVolume = results.getApparentVolume();
    ui->valueTripleMeasureApparentVolume->setText(QString::number(apparentVolume, 'f', 3) + " cm³");

    double openPoresVolume = results.getOpenPoresVolume();
    ui->valueTripleMeasureOpenPoresVolume->setText(QString::number(openPoresVolume, 'f', 3) + " cm³");

    double totalPorosity = results.getTotalPorosity();
    ui->valueTripleMeasureTotalPorosity->setText(QString::number(totalPorosity, 'f', 2) + " %");

    double openPorosity = results.getOpenPorosity();
    ui->valueTripleMeasureOpenPorosity->setText(QString::number(openPorosity, 'f', 2) + " %");

    double closedPorosity = results.getClosedPorosity();
    ui->valueTripleMeasureClosedPorosity->setText(QString::number(closedPorosity, 'f', 2) + " %");

    double waterAbsorption = results.getWaterAbsorption();
    ui->valueTripleMeasureWaterAbsorbability->setText(QString::number(waterAbsorption, 'f', 2) + " %");
}

void MainWindow::updateSaturationMethodPrepareTriple()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    int currentIndex = ui->comboBoxSaturationMethodPrepareMeasureTriple->currentIndex();
    if(currentIndex == -1)
        return;

    SaturationMethod selectedMethod = static_cast<SaturationMethod>(currentIndex);
    radwagMeasureControler->setSaturationMethod(selectedMethod);
}

void MainWindow::updatePrepareSaturationFluidDensityLabel()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    int currentIndex = ui->comboBoxTempFluidPrepareSaturationTrilpe->currentIndex();
    if(currentIndex == -1)
        return;

    double selectedTemperature = ui->comboBoxTempFluidPrepareSaturationTrilpe->itemData(currentIndex).toDouble();

    Fluid currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    const QMap<double, double>& densityTableMap = currentFluid.getDensityTableMap();

    if(!densityTableMap.contains(selectedTemperature))
        return;

    double density = densityTableMap.value(selectedTemperature);
    QString formattedDensity = QString::number(density, 'f', 4) + " g/cm³";
    ui->labelFluidDensityPrepareSaturationTrilpe->setText(formattedDensity);
    radwagMeasureControler->setFluidTemperature(selectedTemperature);
}

void MainWindow::onSpinSaturationTimeChanged()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;
    radwagMeasureControler->setSaturationTime(ui->spinSaturationTimePrepareMeasureTriple->value());
}

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 3 FINISH SECOND ----------------------------------------------------

void MainWindow::fillMeasureSecondLabelsSummary()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    auto measurement = radwagMeasureControler->getActiveMeasure();
    auto results = radwagMeasureControler->calculateResults();

    ui->valueSecondMeasureID->setText(measurement->getSampleId());
    ui->valueSecondMeasureType->setText(measurement->getType() == MeasurementType::TwoStage ? "Dwustopniowy" : "Trzystopniowy");
    ui->valueSecondMeasureOperator->setText(measurement->getAuthor());

    QString dateTime = measurement->getDate().toString("dd-MM-yyyy HH:mm");
    ui->valueSecondMeasureDateTime->setText(dateTime);

    auto sample = measurement->getSample();
    ui->valueSecondSampleName->setText(sample.getName());
    ui->valueSecondMaterial->setText(sample.getMaterialName());
    ui->valueSecondTheoreticalDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 3) + " g/cm³");

    Fluid fluid = measurement->getFluid();
    ui->valueSecondLiquidType->setText(fluid.getName());
    ui->valueSecondFluidTemp->setText(QString::number(measurement->getFluidTemperature()));
    ui->valueSecondLiquidDensity->setText(QString::number(measurement->getFluidDensity(), 'f', 5) + " g/cm³");

    double dryMass = measurement->getSampleDryMass();
    ui->valueSecondMeasureDryMass->setText(QString::number(dryMass, 'f', 3) + " g");

    double massInFluid = measurement->getSampleInFluidMass();
    ui->valueSecondMeasureWetMass->setText(QString::number(massInFluid, 'f', 3) + " g");

    double apparentVolume = results.getApparentVolume();
    ui->valueSecondMeasureApparentVolume->setText(QString::number(apparentVolume, 'f', 3) + " cm³");

    double apparentDensity = results.getApparentDensity();
    ui->valueSecondMeasureApparentDensity->setText(QString::number(apparentDensity, 'f', 3) + " g/cm³");

    double relativeDensity = results.getRelativeDensity();
    ui->valueSecondMeasureRelativeDensity->setText(QString::number(relativeDensity, 'f', 3) + " %");

    double totalPorosity = results.getTotalPorosity();
    ui->valueSecondMeasureTotalPorosity->setText(QString::number(totalPorosity, 'f', 3) + " %");
}

void MainWindow::fillPrepareSaturationDataLabels()
{
    fillSaturationSampleInfoLabels();
    fillSaturationMethodsCombo();
    fillPrepareSaturationFluidInfoLabels();
}

void MainWindow::fillSaturationSampleInfoLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();
    ui->ediSampleIdValuePrepareMeasureTriple->setText(currentSample.getId());
    ui->editSampleMaterialValuePrepareMeasureTriple->setText(currentSample.getMaterialName());

    double airMass = radwagMeasureControler->getDryMass();
    QString formattedAirMass = QString::number(airMass, 'f', 3) + " g";
    ui->editAirWeightValuePrepareMeasureTriple->setText(formattedAirMass);

    ui->editStartSaturationDateTimePrepareMeasureTriple->setMinimumDate(QDate::currentDate());
    ui->editStartSaturationDateTimePrepareMeasureTriple->setDateTime(QDateTime::currentDateTime());
}

void MainWindow::fillSaturationMethodsCombo()
{
    ui->comboBoxSaturationMethodPrepareMeasureTriple->clear();
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::BoilingInWater), static_cast<int>(SaturationMethod::BoilingInWater));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::VacuumMethod), static_cast<int>(SaturationMethod::VacuumMethod));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::LongTermSoaking), static_cast<int>(SaturationMethod::LongTermSoaking));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->setCurrentIndex(-1);
    ui->spinSaturationTimePrepareMeasureTriple->setValue(0);
}

void MainWindow::fillPrepareSaturationFluidInfoLabels()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    ui->editLiquidTypePrepareSaturationTriple->setText(currentFluid.getName());
    fillPrepareSaturationTemperatureCombo();
}

void MainWindow::fillPrepareSaturationTemperatureCombo()
{
    if(!radwagMeasureControler->hasActiveMeasurement())
        return;

    ui->comboBoxTempFluidPrepareSaturationTrilpe->clear();

    const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
    const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

    for(auto it = densityTableMap.keyBegin(); it != densityTableMap.keyEnd(); it++)
        ui->comboBoxTempFluidPrepareSaturationTrilpe->addItem(QString::number(*it) + " °C", *it);

    ui->comboBoxTempFluidPrepareSaturationTrilpe->setCurrentIndex(-1);
    ui->labelFluidDensityPrepareSaturationTrilpe->setText(QString("0.0 g/cm³"));
}

void MainWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel *selectionModel = ui->treeViewLibMeasure->selectionModel();
    int selectedCount = selectionModel->selectedRows().count();
    ui->labelLibSelectedCount->setText(QString("Zaznaczone: %1").arg(selectedCount));
    updateMeasurementCounter();
}

void MainWindow::updateMeasurementCounter()
{
    int allCount = measurementManager->getMeasurements().size();
    ui->labelLibAllCount->setText(QString("Liczba pomiarów: %1").arg(allCount));

    int visibleCount = 0;
    if(measurementProxyModel)
    {
        visibleCount = countVisibleItems(measurementProxyModel, QModelIndex());
    }
    ui->labelLibVisibleCount->setText(QString("Widoczne: %1").arg(visibleCount));

    int selectedCount = ui->treeViewLibMeasure->selectionModel()->selectedRows().count();
    ui->labelLibSelectedCount->setText(QString("Wybrane: %1").arg(selectedCount));
}

int MainWindow::countVisibleItems(QAbstractItemModel* model, const QModelIndex& parent)
{
    int count = 0;
    int rows = model->rowCount(parent);
    for(int i = 0; i < rows; i++)
    {
        QModelIndex index = model->index(i, 0, parent);
        if (model->hasChildren(index))
            count += countVisibleItems(model, index);
        else
            count++;
    }
    return count;
}

void MainWindow::setupLibraryView()
{
    setupLibraryControls();
    setupLibraryModels();
    setupLibraryTreeView();
    updateMeasurementCounter();
}

void MainWindow::setupLibraryControls()
{
    fillComboLibSearchIn();
    fillComboLibGroupBy();
}

void MainWindow::setupLibraryModels()
{
    measurementModel = new MeasurementTreeModel(measurementManager.get(), this);
    measurementProxyModel = new MeasurementSortFilterProxyModel(this);
    measurementProxyModel->setSourceModel(measurementModel);
    measurementProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
}

void MainWindow::setupLibraryTreeView()
{
    ui->treeViewLibMeasure->setModel(measurementProxyModel);

    const int defaultColumnWidth = 100;
    for(int col = MeasurementTreeModel::MeasureType; col < MeasurementTreeModel::ColumnCount; col++)
        ui->treeViewLibMeasure->setColumnWidth(col, defaultColumnWidth);

    ui->treeViewLibMeasure->sortByColumn(MeasurementTreeModel::Date, Qt::DescendingOrder);
    ui->treeViewLibMeasure->setStyleSheet("QTreeView::item:selected { background-color: #0064FF; }");
    ui->treeViewLibMeasure->expandAll();

    connect(ui->treeViewLibMeasure, &QTreeView::doubleClicked, this, &MainWindow::onMeasurementDoubleClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onLibrarySearchTextChanged);
    connect(ui->comboLibSearchIn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLibrarySearchInChanged);
    connect(ui->comboLibGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLibraryGroupByChanged);
    connect(measurementManager.get(), &MeasurementManager::measurementsChanged, this, &MainWindow::refreshLibraryView);
}

void MainWindow::fillComboLibSearchIn()
{
    ui->comboLibSearchIn->clear();
    ui->comboLibSearchIn->addItem("Wszystkie pola", -1);
    ui->comboLibSearchIn->addItem("ID próbki", MeasurementTreeModel::Columns::SampleId);
    ui->comboLibSearchIn->addItem("Nazwa próbki", MeasurementTreeModel::Columns::SampleName);
    ui->comboLibSearchIn->addItem("Materiał", MeasurementTreeModel::Columns::Material);
    ui->comboLibSearchIn->addItem("Ciecz", MeasurementTreeModel::Columns::Fluid);
    ui->comboLibSearchIn->addItem("Status", MeasurementTreeModel::Columns::Status);
    ui->comboLibSearchIn->addItem("Wykonawca", MeasurementTreeModel::Columns::Author);
    ui->comboLibSearchIn->addItem("Data", MeasurementTreeModel::Columns::Date);
}

void MainWindow::fillComboLibGroupBy()
{
    ui->comboLibGroupBy->clear();
    ui->comboLibGroupBy->addItem("Brak grupowania", -1);
    ui->comboLibGroupBy->addItem("Typ pomiaru", MeasurementTreeModel::Columns::MeasureType);
    ui->comboLibGroupBy->addItem("ID próbki", MeasurementTreeModel::Columns::SampleId);
    ui->comboLibGroupBy->addItem("Nazwa próbki", MeasurementTreeModel::Columns::SampleName);
    ui->comboLibGroupBy->addItem("Materiał", MeasurementTreeModel::Columns::Material);
    ui->comboLibGroupBy->addItem("Ciecz", MeasurementTreeModel::Columns::Fluid);
    ui->comboLibGroupBy->addItem("Status", MeasurementTreeModel::Columns::Status);
    ui->comboLibGroupBy->addItem("Wykonawca", MeasurementTreeModel::Columns::Author);
    ui->comboLibGroupBy->addItem("Data", MeasurementTreeModel::Columns::Date);
}

void MainWindow::onLibrarySearchTextChanged(const QString& text)
{
    measurementModel->setFilterText(text);
    ui->treeViewLibMeasure->expandAll();
    updateMeasurementCounter();
}

void MainWindow::onLibrarySearchInChanged(int index)
{
    int columnEnum = ui->comboLibSearchIn->itemData(index).toInt();
    measurementModel->setFilterColumn(columnEnum);
    ui->treeViewLibMeasure->expandAll();
}

void MainWindow::onLibraryGroupByChanged(int index)
{
    int columnEnum = ui->comboLibGroupBy->itemData(index).toInt();
    measurementModel->setGroupBy(columnEnum);
    ui->treeViewLibMeasure->expandAll();
}

void MainWindow::refreshLibraryView()
{
    measurementModel->buildTree();
    ui->treeViewLibMeasure->expandAll();
}

void MainWindow::onMeasurementDoubleClicked(const QModelIndex& index)
{
    // // Pobierz model source (bez proxy)
    // QModelIndex sourceIndex = proxyModel->mapToSource(index);

    // // Sprawdź, czy kliknięto na węzeł pomiarowy (a nie nagłówek grupy)
    // MeasurementTreeModel* model = nullptr;

    // if (ui->treeViewLibMeasure->model() == twoStageProxyModel)
    //     model = twoStageModel;
    // else
    //     model = threeStageModel;

    // // Pobierz ID pomiaru (implementacja zależy od szczegółów MeasurementTreeModel)
    // QString measurementId = model->getMeasurementId(sourceIndex);

    // if (!measurementId.isEmpty()) {
    //     // Otwórz pomiar do edycji lub podglądu
    //     openMeasurement(measurementId);
    // }
}

// Metoda otwierająca pomiar
// void MainWindow::openMeasurement(const QString& measurementId)
// {
//     // Tu implementacja otwierania pomiaru...
//     // Na przykład:
//     auto measurement = measurementManager->getMeasurement(measurementId);
//     if (measurement) {
//         // Otwórz okno edycji/widoku pomiaru
//         // ...
//     }
// }
