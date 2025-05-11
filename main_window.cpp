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
    , measurememntManager(new MeasurementManager(this))
    , radwagMeasureControler(std::make_unique<MeasurementController>(measurememntManager, this))
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

//----------------------------------------------- END TAB DEVICE MANAGER ------------------------------------------


//----------------------------------------------- PAGE INITIAL DATA 0 ------------------------------------------

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
    ui->editLiquidMeasureFinishMeasureSecond->setText(text);
}

void MainWindow::onClearSavedFinishMeasureSecondButtonClicked()
{
    ui->editSavedValueFinishMeasureSecond->clear();
    ui->editLiquidMeasureFinishMeasureSecond->clear();
    radwagMeasureControler->setMassInFluid(0.0);
}

//----------------------------------------------- END PAGE 3 FINISH SECOND -------------------------------------

void MainWindow::navigateToToolBoxPage(QWidget* page)
{
    if(page && ui->stackedWidget->indexOf(page) != -1)
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::goToPreviousMeasureStage()
{
    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    MeasurementStages::Stage prevStage;
    switch(currentStage)
    {
    case MeasurementStages::Stage::StartMeasure:
        break;

    case MeasurementStages::Stage::InitialData:
        prevStage = MeasurementStages::Stage::StartMeasure;
        ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageStartMeasure);
        break;

    case MeasurementStages::Stage::DryMeasure:
        prevStage = MeasurementStages::Stage::InitialData;
        ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
        ui->groupBoxAdditionalSettings->setVisible(true);
        break;

    case MeasurementStages::Stage::PrepareSecond:
    case MeasurementStages::Stage::PrepareTriple:
        prevStage = MeasurementStages::Stage::DryMeasure;
        ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
        break;

    case MeasurementStages::Stage::FinishSecond:
        prevStage = MeasurementStages::Stage::PrepareSecond;
        ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
        break;

    case MeasurementStages::Stage::SaturationMass:
        prevStage = MeasurementStages::Stage::PrepareTriple;
        ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
        break;

    case MeasurementStages::Stage::FinishTriple:
        prevStage = MeasurementStages::Stage::SaturationMass;
        ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
        break;

    default:
        return;
    }

    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
    updateStageLabels();
}

void MainWindow::goToNextMeasureStage()
{
    MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    MeasurementStages::Stage nextStage;
    switch(currentStage)
    {
        case MeasurementStages::Stage::StartMeasure:
            nextStage = MeasurementStages::Stage::InitialData;
            ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);

            if(!radwagMeasureControler->hasActiveMeasurement())
                emit beginNewMeasure();
            radwagMeasureControler->setStage(nextStage);
            break;

        case MeasurementStages::Stage::InitialData:
        {
            if(!validateInitialData())
                return;

            nextStage = MeasurementStages::Stage::DryMeasure;
            ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
            ui->groupBoxAdditionalSettings->setVisible(false);

            if(radwagMeasureControler->setStage(nextStage))
                emit setMeasureInitialData();
            break;
        }
        case MeasurementStages::Stage::DryMeasure:
        {
            if(!vaildateDryMeasureData())
                return;

            if(isTripleMeasurement)
            {
                nextStage = MeasurementStages::Stage::PrepareTriple;
                ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
            }
            else
            {
                nextStage = MeasurementStages::Stage::PrepareSecond;
                ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
            }

            fillPrepareDataLabels();
            radwagMeasureControler->setStage(nextStage);

            break;
        }
        case MeasurementStages::Stage::PrepareSecond:
            if(!checkGuidePrepareMeasureSecondButton())
                return;

            nextStage = MeasurementStages::Stage::FinishSecond;
            ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementSecond);

            radwagMeasureControler->setStage(nextStage);
            fillFinishMeasureSecondLabels();
            break;

        case MeasurementStages::Stage::PrepareTriple:
            nextStage = MeasurementStages::Stage::SaturationMass;
            ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
            radwagMeasureControler->setStage(nextStage);
            break;

        case MeasurementStages::Stage::SaturationMass:
            nextStage = MeasurementStages::Stage::FinishTriple;
            ui->measureDensityStage->setCurrentWidget(ui->pageFinishSaturatedMassTriple);
            radwagMeasureControler->setStage(nextStage);
            break;

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
        ui->editCurrentDryMeasure->setText(QString::number(data.getValue()));
    else if(currentStage == MeasurementStages::Stage::FinishSecond)
        ui->editCurrentValueFinishSecond->setText(QString::number(data.getValue()));
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

void MainWindow::connectButtons()
{
    connectMainNavButtons();
    connectDevicesSettingsButtons();
    connectNavMeasurementButtons();
    connectPrepareWorksationPageButtons();
    connectInitialDataPageButtons();
    connectDryMassPageButtons();
    connectPrepareMeasureSecondPageButtons();
    connectFinishSecondPageButtons();
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
    connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
    connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

    connect(ui->buttonNextPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonNextSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonPrevSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
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

    connect(this, &MainWindow::beginNewMeasure, this, &MainWindow::onBeginNewMeasure);
    connect(this, &MainWindow::setMeasureInitialData, this, &MainWindow::onSetInitialData);
}

void MainWindow::connectPrepareMeasureSecondPageButtons()
{
    connect(ui->buttonConfrimPrepareMeasureSecond, &QPushButton::clicked, this, &MainWindow::onConfrimPrepareMeasureSecondButtonClicked);
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

void MainWindow::finishMeasurement()
{
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    ui->measureDensityStage->setCurrentWidget(isTripleMeasurement ? ui->pageSummaryTriple : ui->pageSummarySecond);
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::Summary));
    if(!isTripleMeasurement)
    {
        fillMeasureSecondLabelsSummary();
        clearInitialDataPage();
        clearDryMeasurePage();
        clearPrepareMeasureSecondPage();
        clearFinishMeasureSecondPage();
    }
    radwagMeasureControler->setStage(MeasurementStages::Stage::Summary);
    updateStageLabels();
    radwagMeasureControler->endMeasure();
}

void MainWindow::setProperty()
{
    ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::StartMeasure));
    ui->scrollAreaInitialData->setBackgroundRole(QPalette::Base);
    ui->scrollArea->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishSecond->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishTriple->setBackgroundRole(QPalette::Base);
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}

//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------

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

bool MainWindow::checkDeviceConnectionWithMessage()
{
    if(!radwagScaleConnector.get() || !radwagScaleConnector->connectionIsActive())
    {
        QMessageBox::warning(this, tr("Brak połączenia"), "Brak połączenia z urządzeniem");
        return false;
    }
    return true;
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

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------


//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------

void MainWindow::updateSaveCurrentDryMeasureButtonState()
{
    ui->buttonSaveCurrentDryMeasure->setEnabled(utils::getDouble(ui->editCurrentDryMeasure->text()) > 0.0);
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

void MainWindow::clearDryMeasurePage()
{
    ui->step1CheckBox->setChecked(false);
    ui->step2CheckBox->setChecked(false);
    ui->step3CheckBox->setChecked(false);

    ui->editCurrentDryMeasure->clear();
    ui->editSavedDryMeasure->clear();
}

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------


//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 2 PREPARE MEASUREMENT ------------------------------------------

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

    int defaultIndex = ui->comboBoxTempFluidPrepareMeasureSecond->findText("20 °C");
    if(defaultIndex != -1)
        ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(defaultIndex);
    else if(!densityTableMap.isEmpty())
        ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(0);
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
    QString formattedDensity = QString::number(density, 'f', 4) + " g/cm³";
    ui->labelLiquidDensityValuePrepareMeasureSecond->setText(formattedDensity);
    radwagMeasureControler->setFluidTemperature(selectedTemperature);
}

void MainWindow::clearPrepareMeasureSecondPage()
{
    ui->stepFirstPrepareMeasureSecond->setChecked(false);
    ui->stepTwoPrepareMeasureSecond->setChecked(false);
    ui->stepTreePrepareMeasureSecond->setChecked(false);
    ui->stepFourPrepareMeasureSecond->setChecked(false);
    ui->stepFivePrepareMeasureSecond->setChecked(false);
    ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(0);
    ui->editSampleIdValuePrepareMeasureSecond->clear();
    ui->editLiquidTypePrepareMeasureSecond->clear();
    ui->editSampleMaterialValuePrepareMeasureSecond->clear();
    ui->labelLiquidDensityValuePrepareMeasureSecond->clear();
    ui->labelAirWeightValuePrepareMeasureSecond->clear();
}

//----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 2 PREPARE MEASUREMENT ------------------------------------------


//----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 3 FINISH SECOND ----------------------------------------------------

void MainWindow::updateSaveFinishSecondButtonState()
{
    ui->buttonSaveCurrentFinishSecond->setEnabled(utils::getDouble(ui->editCurrentValueFinishSecond->text()) > 0.0);
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
    ui->editDensityLiquidFinishMeasureSecond->setText(QString::number(fluidDensity, 'f', 3) + " g/cm³");
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
    ui->valueSecondLiquidDensity->setText(QString::number(measurement->getFluidDensity(), 'f', 3) + " g/cm³");

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


// #include "main_window.h"
// #include "./ui_main_window.h"
// #include "fluid_tabels/fluid_tables_form.h"
// #include "sample/sample_dialog.h"
// #include "tooltip/tooltip_manager.h"
// #include <QMessageBox>
// #include "radwag/measurement.h"
// #include "utils.h"

// MainWindow::MainWindow(QWidget *parent)
//     : QMainWindow(parent)
//     , ui(new Ui::MainWindow)
//     , devicesListModel(this)
//     , devicesListControler(devicesListModel, this)
//     , fluidManager(std::make_unique<FluidManager>(new FluidManager()))
//     , materialManager(std::make_unique<MaterialManager>(new MaterialManager(this)))
//     , sampleManager(std::make_unique<SampleManager>(new SampleManager(this)))
//     , measurememntManager(new MeasurementManager(this))
//     , radwagMeasureControler(std::make_unique<MeasurementController>(measurememntManager, this))
// {
//     initControls();
//     connectButtons();
//     QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

//     // tooltip i.e
//     TooltipManager& tooltipManager = TooltipManager::getInstance();
//     tooltipManager.setGlobalStyle("QToolTip { background-color: #2C3E50; color: white; }");
//     tooltipManager.registerImage("info", ":/icons/image.jpg", 424, 424);
//     tooltipManager.registerTooltip(ui->buttonDryMassExecuteStepOne, ui->buttonDryMassExecuteStepOne->text(), "Wyzeruj wagę wskazanym na ilustracji przyciskiem", "info", TooltipManager::IMAGE_BOTTOM);
// }

// MainWindow::~MainWindow()
// {
//     delete ui;
// }

// //----------------------------------------------- TAB DEVICE MANAGER ------------------------------------------

// void MainWindow::onAddDeviceButtonClicked()
// {
//     devicesListControler.beginNew();
//     if(!deviceConnector.connectionIsActive())
//         fillDevicesCombo();
// }

// void MainWindow::onEditDeviceButtonClicked()
// {
//     auto selectedDeivce = getSelectedDevice();
//     if(!canEditDevice(selectedDeivce))
//     {
//         QMessageBox::warning(this, "Ostrzeżenie", QString("Nie możesz edytować urządzenia z aktywnym połączeniem"));
//         return;
//     }
//     devicesListControler.beginEdit(selectedDeivce);
//     if(!deviceConnector.connectionIsActive())
//         fillDevicesCombo();
// }

// void MainWindow::onRemoveDeviceButtonClicked()
// {
//     auto selectedDeivce = getSelectedDevice();
//     if(!canEditDevice(selectedDeivce))
//     {
//         QMessageBox::warning(this, "Ostrzeżenie", QString("Nie możesz usunąć urządzenia z aktywnym połączeniem"));
//         return;
//     }
//     devicesListControler.beginRemove(selectedDeivce);
//     if(!deviceConnector.connectionIsActive())
//         fillDevicesCombo();
// }

// bool MainWindow::canEditDevice(std::shared_ptr<const Device> &device)
// {
//     return !deviceConnector.connectionIsActive() || (deviceConnector.connectionIsActive() && device != deviceConnector.getActiveDevice());
// }

// std::shared_ptr<const Device> MainWindow::getSelectedDevice()
// {
//     QModelIndex currentIndex = ui->devicesListView->currentIndex();
//     if (!currentIndex.isValid())
//         return nullptr;

//     DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
//     if (!model)
//         return nullptr;

//     return model->getDevice(currentIndex.row());
// }

// //----------------------------------------------- END TAB DEVICE MANAGER ------------------------------------------


// //----------------------------------------------- PAGE INITIAL DATA 0 ------------------------------------------

// void MainWindow::onDeviceComboSelectionChanged()
// {
//     deviceConnector.setActiveDevice(ui->comboBoxSelectDevice->currentData().value<std::shared_ptr<const Device>>());
//     updateConnectonLabelsStatusBar(false);
// }

// void MainWindow::onConnectDeviceClicked()
// {
//     deviceConnector.connectDevice(ui->comboBoxSelectPort->currentText());
// }

// void MainWindow::onDisconnectDeviceClicked()
// {
//     deviceConnector.closeActiveConnection();
//     fillDevicesCombo(true);
// }

// void MainWindow::onSampleComboBoxChanged(int index)
// {
//     upadteSampleEditors();
// }

// void MainWindow::onMeasurementTypeChanged()
// {
//     if(ui->radioMeasureSecond->isChecked())
//         ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageSecondMeasure);
//     else
//         ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageTripleMeasure);
//     updateStageLabels();
// }

// //----------------------------------------------- END PAGE 0 INITIAL DATA ------------------------------------------


// //----------------------------------------------- PAGE 1 DRYMASS ----------------------------------------------------

// void MainWindow::onDryMassExecuteStepOneClicked()
// {
//     if(deviceConnector.connectionIsActive())
//     {
//         deviceConnector.sendZeroCommand();
//         deviceConnector.sendTareCommand();
//         ui->step1CheckBox->setChecked(true);
//     }
//     else
//         QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
// }

// void MainWindow::onDryMassExecuteStepTwoClicked()
// {
//     if(deviceConnector.connectionIsActive())
//     {
//         deviceConnector.sendImmediateWeightCommand();
//         ui->step2CheckBox->setChecked(true);
//     }
//     else
//         QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
// }

// void MainWindow::onDryMassExecuteStepThreeClicked()
// {
//     if(!ui->editCurrentDryMeasure->text().isEmpty())
//     {
//         onSaveCurrentDryMeasureButtonClicked();
//         ui->step3CheckBox->setChecked(true);
//     }
//     else
//         QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
// }

// void MainWindow::onGetCurrentDryMeasureButtonClicked()
// {
//     deviceConnector.sendImmediateWeightCommand();
// }

// void MainWindow::onSaveCurrentDryMeasureButtonClicked()
// {
//     QString text = ui->editCurrentDryMeasure->text().trimmed();
//     if(!radwagMeasureControler->setDryMass(utils::getDouble(text)))
//         QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru!");
//     ui->editSavedDryMeasure->setText(text);
// }

// void MainWindow::onClearSavedDryMeasureButtonClicked()
// {
//     ui->editSavedDryMeasure->clear();
//     radwagMeasureControler->setDryMass(0.0);
// }

// //----------------------------------------------- END PAGE 1 DRYMASS ------------------------------------------


// //----------------------------------------------- PAGE 3 FINISH SECOND ----------------------------------------

// void MainWindow::onFinishMeasureSecondStepTwoClicked()
// {
//     if(deviceConnector.connectionIsActive())
//     {
//         deviceConnector.sendImmediateWeightCommand();
//         ui->step2FinishMeasureSecondCheckBox->setChecked(true);
//     }
//     else
//         QMessageBox::warning(this, "Błąd", "Brak aktywnego połączenia z wagą.");
// }

// void MainWindow::onFinishMeasureSecondStepTreeClicked()
// {
//     if(!ui->editCurrentValueFinishSecond->text().isEmpty())
//     {
//         onSaveCurrentFinishMeasureSecondButtonClicked();
//         ui->step3FinishMeasureSecondCheckBox->setChecked(true);
//     }
//     else
//         QMessageBox::warning(this, "Błąd", "Brak wartości do zapisania.");
// }

// void MainWindow::onGetCurrentFinishMeasureSecondButtonClicked()
// {
//     deviceConnector.sendImmediateWeightCommand();
// }

// void MainWindow::onSaveCurrentFinishMeasureSecondButtonClicked()
// {
//     QString text = ui->editCurrentValueFinishSecond->text().trimmed();
//     if(!radwagMeasureControler->setMassInFluid(utils::getDouble(text)))
//         QMessageBox::warning(this, tr("Błąd"), "Błąd zapisu pomiaru! FinishSecond");
//     ui->editSavedValueFinishMeasureSecond->setText(text);
// }

// void MainWindow::onClearSavedFinishMeasureSecondButtonClicked()
// {
//     ui->editSavedValueFinishMeasureSecond->clear();
//     radwagMeasureControler->setMassInFluid(0.0);
// }

// void MainWindow::onCalculatelabelVolumeFinishMeasureSecondButtonClicked()
// {
//     if(!radwagMeasureControler->canCalculateResult())
//     {
//         QMessageBox::warning(this, tr("Błąd"), "Brak danych niezbędnych do wykonania obliczeń!");
//         return;
//     }

//     auto results = radwagMeasureControler->calculateResults();

//     double apparentVolume = results.getApparentVolume();
//     double apparentDensity = results.getApparentDensity();
//     double relativeDensity = results.getRelativeDensity();

//     ui->editVolumeFinishMeasureSecond->setText(QString::number(apparentVolume, 'f', 2) + " cm³");
//     ui->editDensityApparentFinishMeasureSecond->setText(QString::number(apparentDensity, 'f', 3) + " g/cm³");
//     ui->editVolumeRelativeFinishMeasureSecond->setText(QString::number(relativeDensity, 'f', 2) + " %");
// }

// //----------------------------------------------- END PAGE 3 FINISH SECOND -------------------------------------

// void MainWindow::navigateToToolBoxPage(QWidget* page)
// {
//     if(page && ui->stackedWidget->indexOf(page) != -1)
//         ui->stackedWidget->setCurrentWidget(page);
// }

// void MainWindow::goToPreviousMeasureStage()
// {
//     MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
//     bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

//     // Określenie poprzedniego etapu pomiaru
//     MeasurementStages::Stage prevStage;
//     switch(currentStage)
//     {
//     case MeasurementStages::Stage::None:
//         break;

//     case MeasurementStages::Stage::InitialData:
//         prevStage = MeasurementStages::Stage::None;
//         ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageStartMeasure);
//         break;

//     case MeasurementStages::Stage::DryMeasure:
//         prevStage = MeasurementStages::Stage::InitialData;
//         ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
//         ui->groupBoxAdditionalSettings->setVisible(true);
//         break;

//     case MeasurementStages::Stage::PrepareSecond:
//     case MeasurementStages::Stage::PrepareTriple:
//         prevStage = MeasurementStages::Stage::DryMeasure;
//         ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
//         break;

//     case MeasurementStages::Stage::FinishSecond:
//         prevStage = MeasurementStages::Stage::PrepareSecond;
//         ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
//         break;

//     case MeasurementStages::Stage::SaturationMass:
//         prevStage = MeasurementStages::Stage::PrepareTriple;
//         ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
//         break;

//     case MeasurementStages::Stage::FinishTriple:
//         prevStage = MeasurementStages::Stage::SaturationMass;
//         ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
//         break;

//     default:
//         // Dla pierwszego etapu nie ma już poprzedniej strony
//         return;
//     }

//     // Zapisanie aktualnego etapu pomiaru
//     ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
//     updateStageLabels();
// }

// void MainWindow::goToNextMeasureStage()
// {
//     MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
//     bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

//     MeasurementStages::Stage nextStage;
//     switch(currentStage)
//     {
//         case MeasurementStages::Stage::None:
//             nextStage = MeasurementStages::Stage::InitialData;
//             ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
//             break;

//         case MeasurementStages::Stage::InitialData:
//         {
//             if(!validateInitialData())
//                 return;

//             if(!radwagMeasureControler->hasActiveMeasurement())
//                 emit beginNewMeasure();

//             nextStage = MeasurementStages::Stage::DryMeasure;
//             ui->measureDensityStage->setCurrentWidget(ui->pageDryMeasure);
//             ui->groupBoxAdditionalSettings->setVisible(false);
//             radwagMeasureControler->setStage(nextStage);
//             break;
//         }
//         case MeasurementStages::Stage::DryMeasure:
//         {
//             if(!vaildateDryMeasureData())
//                 return;

//             if(isTripleMeasurement)
//             {
//                 nextStage = MeasurementStages::Stage::PrepareTriple;
//                 ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureTriple);
//             }
//             else
//             {
//                 nextStage = MeasurementStages::Stage::PrepareSecond;
//                 ui->measureDensityStage->setCurrentWidget(ui->pagePrepareMeasureSecond);
//             }
//             radwagMeasureControler->setStage(nextStage);
//             fillPrepareDataLabels();
//             break;
//         }
//         case MeasurementStages::Stage::PrepareSecond:
//             nextStage = MeasurementStages::Stage::FinishSecond;
//             ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementSecond);

//             fillFinishMeasureSecondLabels();
//             radwagMeasureControler->setStage(nextStage);
//             break;

//         case MeasurementStages::Stage::PrepareTriple:
//             nextStage = MeasurementStages::Stage::SaturationMass;
//             ui->measureDensityStage->setCurrentWidget(ui->pageSatruationMassTriple);
//             radwagMeasureControler->setStage(nextStage);
//             break;

//         case MeasurementStages::Stage::SaturationMass:
//             nextStage = MeasurementStages::Stage::FinishTriple;
//             ui->measureDensityStage->setCurrentWidget(ui->pageFinishSaturatedMassTriple);
//             radwagMeasureControler->setStage(nextStage);
//             break;

//         // case MeasurementStages::Stage::SaturatedMass:
//         //     nextStage = MeasurementStage::SaturatedMass;
//         //     ui->measureDensityStage->setCurrentWidget(ui->pageFinishMeasurementTriple);
//         //     break;


//         default:
//             // Dla ostatnich etapów nie ma już następnej strony
//             return;
//     }

//     // Zapisanie aktualnego etapu pomiaru
//     ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(nextStage));
//     updateStageLabels();
// }

// void MainWindow::onMainPageChanged(int index)
// {
//     updateActionIcons(index);
// }

// void MainWindow::updateActionIcons(int index)
// {
//     ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
//     ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
// }

// void MainWindow::buttonTableFluidsOnClicked()
// {
//     QMap<QString, Fluid> fluids = fluidManager->getFluids();
//     auto dialog = new FluidTablesDialog(fluids, this);
//     dialog->exec();
//     fluidManager->setFluids(fluids);
//     fillFluidCombo();
// }

// void MainWindow::buttonSamplesOnClicked()
// {
//     QMap<QString, Sample> samples = sampleManager->getSamples();
//     QMap<QString, Material> materials = materialManager->getMaterials();
//     auto dialog = new SampleDialog(samples, materials, this);
//     connect(dialog, &SampleDialog::materialsChanged, this, &MainWindow::onMaterialsChanged);
//     dialog->exec();
//     sampleManager->setSamples(samples);
//     fillSampleCombo();
//     upadteSampleEditors();
// }

// void MainWindow::onMaterialsChanged(const QMap<QString, Material> &materials)
// {
//     materialManager->setMaterials(materials);
// }

// void MainWindow::onBeginNewMeasure()
// {
//     MeasurementType type = ui->radioMeasureSecond->isChecked() ? MeasurementType::TwoStage : MeasurementType::ThreeStage;
//     auto sample = sampleManager->getSample(ui->comboBoxSampleSelection->currentData().toString());
//     auto fluid = fluidManager->getFluid(ui->comboBoxFluid->currentText());
//     QString author = ui->editAuthor->text().trimmed();
//     radwagMeasureControler->beginNewMeasure(type, sample, fluid, author);
// }

// void MainWindow::onRadwagMeasueReady(const RadwagMeasure &data)
// {
//     MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
//     if(currentStage == MeasurementStages::Stage::DryMeasure)
//         ui->editCurrentDryMeasure->setText(QString::number(data.getValue()));
//     else if(currentStage == MeasurementStages::Stage::FinishSecond)
//     {
//         ui->editCurrentValueFinishSecond->setText(QString::number(data.getValue()));
//         ui->editLiquidMeasureFinishMeasureSecond->setText(QString::number(data.getValue()));
//     }
// }

// void MainWindow::initControls()
// {
//     ui->setupUi(this);
//     ui->statusbar->addPermanentWidget(ui->labelDeviceNameStatusBar);
//     ui->statusbar->addPermanentWidget(ui->labelConnectionStatusStatusBar);
//     setIcons();
//     setProperty();
//     navigateToToolBoxPage(ui->measureDensityPage);
//     updateStageLabels();
//     updateActionIcons(0);
//     ui->devicesListView->setModel(&devicesListModel);
//     fillDevicesCombo();
//     fillSerialPortCombo();
//     fillFluidCombo();
//     fillSampleCombo();
//     onMeasurementTypeChanged();
//     updateStatusConnectionLabel(false);
//     updateConnectonLabelsStatusBar(false);
//     updateSaveCurrentDryMeasureButtonState();
// }

// void MainWindow::onConnectResult(bool connected)
// {
//     updateStatusConnectionLabel(connected);
//     if(connected)
//     {
//         // deviceConnector.stopContinuousTransmissionBasicUnit();
//         // deviceConnector.stopContinuousTransmissionCurrentUnit();
//         deviceConnector.startContinuousTransmissionCurrentUnit();
//     }
// }

// void MainWindow::onStartMeasureButtonClicked()
// {
//     if(!checkDeviceConnectionWithMessage())
//         return;

//     if(!ui->checkBoxStep1PrepareWorkstation->isChecked() ||
//        !ui->checkBoxStep2PrepareWorkstation->isChecked() ||
//        !ui->checkBoxStep3PrepareWorkstation->isChecked() ||
//        !ui->checkBoxStep4PrepareWorkstation->isChecked() ||
//        !ui->checkBoxStep5PrepareWorkstation->isChecked())
//     {
//         QMessageBox::warning(this, "Niepełne przygotowanie", "Przed rozpoczęciem pomiaru wykonaj wszystkie kroki przygotowawcze.");
//         return;
//     }

//     ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageMeasureProcess);
//     goToNextMeasureStage();
// }

// void MainWindow::onShowHydroSetSchemeButtonClicked()
// {
//     QDialog *schemeDialog = new QDialog(this);
//     schemeDialog->setWindowTitle("Schemat zestawu do wyznaczania gęstości ciał stałych");

//     QVBoxLayout *mainLayout = new QVBoxLayout(schemeDialog);

//     QLabel *imageLabel = new QLabel(schemeDialog);
//     QPixmap schemeImage(":/schema_img/schema_density_solids.png");
//     if (schemeImage.isNull())
//     {
//         imageLabel->setText("Nie można załadować obrazka!");
//     }
//     else
//     {
//         QLabel *noteLabel = new QLabel(schemeDialog);
//         noteLabel->setText("Elementy oznaczone <font color='yellow'>kolorem żółtym </font> są niezbędne do przeprowadzenia właściwego pomiaru.");
//         noteLabel->setAlignment(Qt::AlignCenter);

//         QPalette pal = noteLabel->palette();
//         pal.setColor(QPalette::Window, QColor(240, 240, 240));
//         noteLabel->setAutoFillBackground(false);
//         noteLabel->setPalette(pal);
//         noteLabel->setFrameShape(QFrame::Box);
//         noteLabel->setFrameShadow(QFrame::Sunken);
//         noteLabel->setLineWidth(1);
//         noteLabel->setMargin(8);

//         mainLayout->addWidget(noteLabel);

//         QScreen *screen = QGuiApplication::primaryScreen();
//         QRect screenGeometry = screen->geometry();
//         int maxWidth = screenGeometry.width() * 0.8;  // 80% szerokości ekranu
//         int maxHeight = screenGeometry.height() * 0.8;  // 80% wysokości ekranu
//         if(schemeImage.width() > maxWidth || schemeImage.height() > maxHeight)
//             schemeImage = schemeImage.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//         imageLabel->setPixmap(schemeImage);
//     }

//     mainLayout->addWidget(imageLabel);
//     schemeDialog->setLayout(mainLayout);

//     schemeDialog->exec();
//     delete schemeDialog;
// }

// void MainWindow::onConfrimPrepareWorkstationButtonClicked()
// {
//     ui->checkBoxStep1PrepareWorkstation->setChecked(true);
//     ui->checkBoxStep2PrepareWorkstation->setChecked(true);
//     ui->checkBoxStep3PrepareWorkstation->setChecked(true);
//     ui->checkBoxStep4PrepareWorkstation->setChecked(true);
//     ui->checkBoxStep5PrepareWorkstation->setChecked(true);
// }

// void MainWindow::connectButtons()
// {
//     connectMainNavButtons();
//     connectDevicesSettingsButtons();
//     connectNavMeasurementButtons();
//     connectPrepareWorksationPageButtons();
//     connectInitialDataPageButtons();
//     connectDryMassPageButtons();
//     connectFinishSecondPageButtons();
//     connectCatalogsButtons();
// }

// void MainWindow::connectMainNavButtons()
// {
//     connect(ui->actionSettings, &QAction::triggered, this, [this]() {
//         navigateToToolBoxPage(ui->settingsPage);
//     });

//     connect(ui->actionMeasureDensity, &QAction::triggered, this, [this]() {
//         navigateToToolBoxPage(ui->measureDensityPage);
//     });

//     connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onMainPageChanged);
// }

// void MainWindow::connectNavMeasurementButtons()
// {
//     connect(ui->buttonPrevData, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
//     connect(ui->buttonNextData, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);

//     connect(&deviceConnector, &DeviceConnector::radwagDataReady, this, &MainWindow::onRadwagMeasueReady);

//     connect(ui->buttonNextDryMass, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
//     connect(ui->buttonPrevDryMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
//     connect(ui->buttonNextPreparation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
//     connect(ui->buttonPrevPreparation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
//     connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
//     connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);

//     connect(ui->buttonNextPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
//     connect(ui->buttonPrevPrepareSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
//     connect(ui->buttonNextSaturation, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
//     connect(ui->buttonPrevSaturation, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
//     connect(ui->buttonFinishMeasurementTriple, &QPushButton::clicked, this, &MainWindow::finishMeasurement);
//     connect(ui->buttonPrevSaturatedMass, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
// }

// void MainWindow::connectPrepareWorksationPageButtons()
// {
//     connect(ui->buttonShowHydroSetScheme, &QPushButton::clicked, this, &MainWindow::onShowHydroSetSchemeButtonClicked);
//     connect(ui->buttonConfrimPrepareWorkstation, &QPushButton::clicked, this, &MainWindow::onConfrimPrepareWorkstationButtonClicked);
//     connect(ui->buttonStartMeasure, &QPushButton::clicked, this, &MainWindow::onStartMeasureButtonClicked);
// }

// void MainWindow::connectInitialDataPageButtons()
// {
//     connect(ui->comboBoxSelectDevice, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
//     connect(ui->buttonConnectDevice, &QPushButton::clicked, this, &MainWindow::onConnectDeviceClicked);
//     connect(ui->buttonDisconnectDevice, &QPushButton::clicked, this, &MainWindow::onDisconnectDeviceClicked);
//     connect(&deviceConnector, &DeviceConnector::connectionResult, this, &MainWindow::onConnectResult);

//     connect(ui->radioMeasureSecond, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);
//     connect(ui->radioMeasureTriple, &QRadioButton::toggled, this, &MainWindow::onMeasurementTypeChanged);
//     connect(ui->comboBoxSampleSelection, &QComboBox::currentIndexChanged, this, &MainWindow::onSampleComboBoxChanged);

//     connect(this, &MainWindow::beginNewMeasure, this, &MainWindow::onBeginNewMeasure);
// }

// void MainWindow::connectDryMassPageButtons()
// {
//     connect(ui->buttonDryMassExecuteStepOne, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepOneClicked);
//     connect(ui->buttonDryMassExecuteStepTwo, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepTwoClicked);
//     connect(ui->buttonDryMassExecuteStepThree, &QPushButton::clicked, this, &MainWindow::onDryMassExecuteStepThreeClicked);

//     connect(ui->buttonGetCurrentDryMeasure, &QPushButton::clicked, this, &MainWindow::onGetCurrentDryMeasureButtonClicked);
//     connect(ui->buttonSaveCurrentDryMeasure, &QPushButton::clicked, this, &MainWindow::onSaveCurrentDryMeasureButtonClicked);
//     connect(ui->buttonClearEditSavedDryMeasure, &QPushButton::clicked, this, &MainWindow::onClearSavedDryMeasureButtonClicked);

//     connect(ui->editCurrentDryMeasure, &QLineEdit::textChanged, this, &MainWindow::updateSaveCurrentDryMeasureButtonState);
// }

// void MainWindow::connectFinishSecondPageButtons()
// {
//     connect(ui->buttonFinishMeasureSecondExecuteStepTwo, &QPushButton::clicked, this, &MainWindow::onFinishMeasureSecondStepTwoClicked);
//     connect(ui->buttonFinishMeasureSecondExecuteStepTree, &QPushButton::clicked, this, &MainWindow::onFinishMeasureSecondStepTreeClicked);

//     connect(ui->buttonGetCurrentValueFinishMeasureSecond, &QPushButton::clicked, this, &MainWindow::onGetCurrentFinishMeasureSecondButtonClicked);
//     connect(ui->buttonSaveCurrentFinishSecond, &QPushButton::clicked, this, &MainWindow::onSaveCurrentFinishMeasureSecondButtonClicked);
//     connect(ui->buttonClearSavedValueFinishMeasureSecond, &QPushButton::clicked, this, &MainWindow::onClearSavedFinishMeasureSecondButtonClicked);
//     connect(ui->buttonCalculatelabelVolumeFinishMeasureSecond, &QPushButton::clicked, this, &MainWindow::onCalculatelabelVolumeFinishMeasureSecondButtonClicked);

//     connect(ui->editCurrentValueFinishSecond, &QLineEdit::textChanged, this, &MainWindow::updateSaveFinishSecondButtonState);
// }

// void MainWindow::connectCatalogsButtons()
// {
//     connect(ui->buttonTableFluids, &QPushButton::clicked, this, &MainWindow::buttonTableFluidsOnClicked);
//     connect(ui->buttonSamples, &QPushButton::clicked, this, &MainWindow::buttonSamplesOnClicked);
// }

// void MainWindow::connectDevicesSettingsButtons()
// {
//     connect(ui->addDeviceButton, &QPushButton::clicked, this, &MainWindow::onAddDeviceButtonClicked);
//     connect(ui->editDeviceButton, &QPushButton::clicked, this, &MainWindow::onEditDeviceButtonClicked);
//     connect(ui->deleteDeviceButton, &QPushButton::clicked, this, &MainWindow::onRemoveDeviceButtonClicked);
// }

// void MainWindow::updateStageLabels()
// {
//     QFont normalFont;
//     normalFont.setBold(false);
//     normalFont.setPixelSize(12);

//     QFont boldFont = normalFont;
//     boldFont.setBold(true);
//     boldFont.setPixelSize(13);

//     QPalette normalPalette;
//     QPalette activePalette;
//     activePalette.setColor(QPalette::WindowText, ACTIVE_LABEL_COLOR);

//     MeasurementStages::Stage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStages::Stage>();
//     bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

//     if(isTripleMeasurement)
//     {
//         QList<QLabel*> tripleLabels =
//         {
//             ui->labelStageDataTriple,
//             ui->labelStageDryMassTriple,
//             ui->labelStagePrepareSaturation,
//             ui->labelStageSaturationMass,
//             ui->labelStageSaturatedMass,
//             ui->labelStageSummaryTriple
//         };

//         for (QLabel* label : tripleLabels)
//         {
//             label->setFont(normalFont);
//             label->setPalette(normalPalette);
//         }

//         switch (currentStage)
//         {
//             case MeasurementStages::Stage::InitialData:
//                 ui->labelStageDataTriple->setFont(boldFont);
//                 ui->labelStageDataTriple->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::DryMeasure:
//                 ui->labelStageDryMassTriple->setFont(boldFont);
//                 ui->labelStageDryMassTriple->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::PrepareTriple:
//                 ui->labelStagePrepareSaturation->setFont(boldFont);
//                 ui->labelStagePrepareSaturation->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::SaturationMass:
//                 ui->labelStageSaturationMass->setFont(boldFont);
//                 ui->labelStageSaturationMass->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::FinishTriple:
//                 ui->labelStageSaturatedMass->setFont(boldFont);
//                 ui->labelStageSaturatedMass->setPalette(activePalette);
//                 break;
//             default:
//                 break;
//         }
//     }
//     else
//     {
//         QList<QLabel*> secondLabels =
//         {
//             ui->labelStageData,
//             ui->labelStageDryMass,
//             ui->labelStagePreparation,
//             ui->labelStageFluidMass,
//             ui->labelStageSummary
//         };

//         for(QLabel* label : secondLabels)
//         {
//             label->setFont(normalFont);
//             label->setPalette(normalPalette);
//         }

//         switch (currentStage)
//         {
//             case MeasurementStages::Stage::InitialData:
//                 ui->labelStageData->setFont(boldFont);
//                 ui->labelStageData->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::DryMeasure:
//                 ui->labelStageDryMass->setFont(boldFont);
//                 ui->labelStageDryMass->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::PrepareSecond:
//                 ui->labelStagePreparation->setFont(boldFont);
//                 ui->labelStagePreparation->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::SaturationMass:
//                 ui->labelStagePreparation->setFont(boldFont);
//                 ui->labelStagePreparation->setPalette(activePalette);
//                 break;
//             case MeasurementStages::Stage::FinishSecond:
//                 ui->labelStageFluidMass->setFont(boldFont);
//                 ui->labelStageFluidMass->setPalette(activePalette);
//                 break;
//             default:
//                 break;
//         }
//     }
// }

// void MainWindow::finishMeasurement()
// {
//     bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

//     // Tutaj można dodać logikę zapisywania wyników pomiaru

//     // Informacja o zakończeniu pomiaru
//     QString message = isTripleMeasurement ?
//                           tr("Pomiar trzystopniowy zakończony pomyślnie.") :
//                           tr("Pomiar dwustopniowy zakończony pomyślnie.");

//     QMessageBox::information(this, tr("Pomiar zakończony"), message);

//     // Powrót do pierwszej strony
//     ui->measureDensityStage->setCurrentWidget(ui->pageInitialData);
//     ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::None));
//     updateStageLabels();
//     radwagMeasureControler->endMeasure();
// }

// void MainWindow::setProperty()
// {
//     ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::None));
//     ui->scrollAreaInitialData->setBackgroundRole(QPalette::Base);
//     ui->scrollArea->setBackgroundRole(QPalette::Base);
// }

// void MainWindow::setIcons()
// {
//     defaultSettingsIcon = QIcon(":/icons/settings_white.png");
//     activeSettingsIcon = QIcon(":/icons/settings_selected.png");
//     defaultRadwagIcon = QIcon(":/icons/balance_white.png");
//     activeRadwagIcon = QIcon(":/icons/balance_selected.png");
// }

// //----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------

// void MainWindow::fillDevicesCombo(bool keepActiveDevice)
// {
//     DeviceListModel* model = qobject_cast<DeviceListModel*>(ui->devicesListView->model());
//     if(!model)
//         return;

//     ui->comboBoxSelectDevice->blockSignals(true);
//     const QList<std::shared_ptr<const Device>>& devicesList = model->getDevicesList();

//     QString currentDeviceName;
//     if(keepActiveDevice)
//         currentDeviceName = ui->comboBoxSelectDevice->currentText();
//     ui->comboBoxSelectDevice->clear();

//     for(const auto& device : devicesList)
//     {
//         auto deviceType = device->getDeviceType();
//         if(deviceType != DeviceType::RadwagScaleAC220 && deviceType != DeviceType::RadwagScaleAC350)
//             continue;

//         QVariant deviceData;
//         deviceData.setValue(device);
//         ui->comboBoxSelectDevice->addItem(device->getName(), deviceData);
//     }
//     if(currentDeviceName.isEmpty())
//         ui->comboBoxSelectDevice->setCurrentIndex(-1);
//     else
//         ui->comboBoxSelectDevice->setCurrentText(currentDeviceName);
//     onDeviceComboSelectionChanged();
//     ui->comboBoxSelectDevice->blockSignals(false);
// }

// void MainWindow::fillSerialPortCombo()
// {
//     ui->comboBoxSelectPort->addItems(deviceConnector.getAvailablePorts());
// }

// void MainWindow::fillSampleCombo()
// {
//     ui->comboBoxSampleSelection->clear();
//     const QMap<QString, Sample> &samples = sampleManager->getSamples();
//     for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
//     {
//         QString id = it.value().getId();
//         QString name = it.value().getName();
//         ui->comboBoxSampleSelection->addItem(it.value().getName(), it.value().getId());
//     }
//     ui->comboBoxSampleSelection->setCurrentIndex(-1);
// }

// void MainWindow::fillFluidCombo()
// {
//     ui->comboBoxFluid->clear();
//     const QMap<QString, Fluid> &fluids = fluidManager->getFluids();
//     for(auto &fluid : fluids)
//         ui->comboBoxFluid->addItem(fluid.getName());
//     ui->comboBoxFluid->setCurrentIndex(-1);
// }

// void MainWindow::updateStatusConnectionLabel(bool connectionStatus)
// {
//     if(connectionStatus)
//     {
//         ui->labelEditStatusConnection->setStyleSheet("color: green; font-weight: bold;");
//         ui->labelEditStatusConnection->setText("Połączono");
//         ui->comboBoxSelectDevice->setEnabled(false);
//     }
//     else
//     {
//         ui->labelEditStatusConnection->setStyleSheet("color: red; font-weight: bold;");
//         ui->labelEditStatusConnection->setText("Brak połączenia");
//         ui->comboBoxSelectDevice->setEnabled(true);
//     }
//     updateConnectonLabelsStatusBar(connectionStatus);
// }

// void MainWindow::updateConnectonLabelsStatusBar(bool connectionStatus)
// {
//     if(connectionStatus)
//     {
//         ui->labelDeviceNameStatusBar->setText("Urządzenie: " + deviceConnector.getActiveDevice()->getName());
//         ui->labelConnectionStatusStatusBar->setText("Status: <font color='green'><b>Połączono</b></font>");
//         ui->comboBoxSelectDevice->setEnabled(false);
//     }
//     else
//     {
//         QString deviceName = ui->comboBoxSelectDevice->currentText();
//         ui->labelDeviceNameStatusBar->setText("Urządzenie: " + (deviceName.isEmpty() ? "Nie wybrano" : ui->comboBoxSelectDevice->currentText()));
//         ui->labelConnectionStatusStatusBar->setText("Status: <font color='red'><b>Brak połączenia</b></font>");
//         ui->comboBoxSelectDevice->setEnabled(true);
//     }
// }

// void MainWindow::upadteSampleEditors()
// {
//     clearSampleEditors();

//     QString smapleId = ui->comboBoxSampleSelection->currentData().toString();
//     Sample sample = sampleManager->getSample(smapleId);
//     if(sample.getId().isEmpty())
//         return;

//     ui->editSampleId->setText(smapleId);
//     ui->editSampleName->setText(sample.getName());
//     ui->editMaterial->setText(sample.getMaterialName());
//     ui->editMaterialDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 3) +  " g/cm³");
//     ui->editSampleDescription->setPlainText(sample.getDescription());
// }

// void MainWindow::clearSampleEditors()
// {
//     ui->editSampleId->clear();
//     ui->editSampleName->clear();
//     ui->editMaterial->clear();
//     ui->editMaterialDensity->clear();
//     ui->editSampleDescription->clear();
// }

// bool MainWindow::checkDeviceConnectionWithMessage()
// {
//     if(!deviceConnector.connectionIsActive())
//     {
//         QMessageBox::warning(this, tr("Brak połączenia"), "Brak połączenia z urządzeniem");
//         return false;
//     }
//     return true;
// }

// bool MainWindow::validateInitialData()
// {
//     if(!checkDeviceConnectionWithMessage())
//         return false;

//     if(ui->comboBoxSampleSelection->currentText().isEmpty())
//     {
//         QMessageBox::warning(this, tr("Brak wybranej próbki"), "Brak wybranej próbki");
//         return false;
//     }

//     if(ui->editAuthor->text().isEmpty())
//     {
//         QMessageBox::warning(this, tr("Brak danych"), "Pole autor nie może być puste");
//         return false;
//     }

//     return true;
// }

// //----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 0 INITIAL DATA ------------------------------------------


// //----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------

// void MainWindow::updateSaveCurrentDryMeasureButtonState()
// {
//     ui->buttonSaveCurrentDryMeasure->setEnabled(utils::getDouble(ui->editCurrentDryMeasure->text()) > 0.0);
// }

// bool MainWindow::vaildateDryMeasureData()
// {
//     if(!deviceConnector.connectionIsActive())
//     {
//         QMessageBox::warning(this, tr("Brak połączenia"), "Brak połączenia z urządzeniem");
//         return false;
//     }

//     if(ui->editSavedDryMeasure->text().isEmpty())
//     {
//         QMessageBox::warning(this, tr("Brak danych"), "Aby przejść dalej musisz zapisać pomiar");
//         return false;
//     }

//     return true;
// }

// //----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 1 DRY MASS ------------------------------------------


// //----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 2 PREPARE MEASUREMENT ------------------------------------------

// void MainWindow::fillPrepareDataLabels()
// {
//     fillSampleInfoLabels();
//     fillFluidInfoLabels();
// }

// void MainWindow::fillSampleInfoLabels()
// {
//     if(!radwagMeasureControler->hasActiveMeasurement())
//         return;

//     const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();

//     ui->editSampleIdValuePrepareMeasureSecond->setText(currentSample.getId());
//     ui->editSampleMaterialValuePrepareMeasureSecond->setText(currentSample.getMaterialName());

//     double airMass = radwagMeasureControler->getDryMass();
//     QString formattedAirMass = QString::number(airMass, 'f', 3) + " g";
//     ui->labelAirWeightValuePrepareMeasureSecond->setText(formattedAirMass);
// }

// void MainWindow::fillFluidInfoLabels()
// {
//     if(!radwagMeasureControler->hasActiveMeasurement())
//         return;

//     const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
//     ui->editLiquidTypePrepareMeasureSecond->setText(currentFluid.getName());
//     fillTemperatureComboBox();
//     updateFluidDensityLabel();

//     connect(ui->comboBoxTempFluidPrepareMeasureSecond, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateFluidDensityLabel);
// }

// void MainWindow::fillTemperatureComboBox()
// {
//     if(!radwagMeasureControler->hasActiveMeasurement())
//         return;

//     ui->comboBoxTempFluidPrepareMeasureSecond->clear();

//     const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
//     const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

//     for(auto it = densityTableMap.keyBegin(); it != densityTableMap.keyEnd(); it++)
//         ui->comboBoxTempFluidPrepareMeasureSecond->addItem(QString::number(*it) + " °C", *it);

//     int defaultIndex = ui->comboBoxTempFluidPrepareMeasureSecond->findText("20 °C");
//     if(defaultIndex != -1)
//         ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(defaultIndex);
//     else if(!densityTableMap.isEmpty())
//         ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(0);
// }

// void MainWindow::updateFluidDensityLabel()
// {
//     if(!radwagMeasureControler->hasActiveMeasurement())
//         return;

//     int currentIndex = ui->comboBoxTempFluidPrepareMeasureSecond->currentIndex();
//     if(currentIndex == -1)
//         return;

//     double selectedTemperature = ui->comboBoxTempFluidPrepareMeasureSecond->itemData(currentIndex).toDouble();

//     Fluid currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();
//     const QMap<double, double>& densityTableMap = currentFluid.getDensityTableMap();

//     if(!densityTableMap.contains(selectedTemperature))
//         return;

//     double density = densityTableMap.value(selectedTemperature);
//     QString formattedDensity = QString::number(density, 'f', 4) + " g/cm³";
//     ui->labelLiquidDensityValuePrepareMeasureSecond->setText(formattedDensity);
//     radwagMeasureControler->setFluidTemperature(selectedTemperature);
// }

// //----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 2 PREPARE MEASUREMENT ------------------------------------------


// //----------------------------------------------- FILLING AND HELPER METHODS FOR PAGE 3 FINISH SECOND ----------------------------------------------------

// void MainWindow::updateSaveFinishSecondButtonState()
// {
//     ui->buttonSaveCurrentFinishSecond->setEnabled(utils::getDouble(ui->editCurrentValueFinishSecond->text()) > 0.0);
// }

// void MainWindow::fillFinishMeasureSecondLabels()
// {
//     if(!radwagMeasureControler->hasActiveMeasurement())
//         return;

//     const Sample &currentSample = radwagMeasureControler->getActiveMeasure()->getSample();
//     const Fluid &currentFluid = radwagMeasureControler->getActiveMeasure()->getFluid();

//     ui->editSampleIdValueFinishMeasureSecond->setText(currentSample.getId());
//     ui->editSampleMaterialValueFinishMeasureSecond->setText(currentSample.getMaterialName());
//     ui->editLiquidFinishMeasureSecond->setText(currentFluid.getName());

//     double airMass = radwagMeasureControler->getDryMass();
//     double fluidDensity = radwagMeasureControler->getFluidDensity();

//     ui->editDryMeasureFinishMeasureSecond->setText(QString::number(airMass, 'f', 3) + " g");
//     ui->editDensityLiquidFinishMeasureSecond->setText(QString::number(fluidDensity, 'f', 3) + " g/cm³");
// }

// //----------------------------------------------- END FILLING AND HELPER METHODS FOR PAGE 3 FINISH SECOND ----------------------------------------------------
