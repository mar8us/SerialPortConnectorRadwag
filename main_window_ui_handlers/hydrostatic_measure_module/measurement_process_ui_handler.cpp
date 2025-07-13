#include "measurement_process_ui_handler.h"
#include <QDialog>

#include "../../main_window.h"
#include "../../sample/sample_dialog.h"
#include "../../fluid_tabels/fluid_tables_form.h"
#include "../../app_core.h"
#include "../../utils.h"
#include "measurement_state_machine.h"

MeasurementProcessUiHandler::MeasurementProcessUiHandler(MainWindow *mainWindow, MeasurementController &radwagMeasureControler, HydrostaticDataHolder &dataHolder, QObject *parent)
    : QObject(parent)
    , radwagMeasureControler(radwagMeasureControler)
    , dataHolder(dataHolder)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , measureStateMachine(nullptr)
{

}

void MeasurementProcessUiHandler::initialize()
{
    initializeMappings();
    initializeStateMachine();
    setupCharts();
    updateSaveCurrentDryMeasureButtonState();

    connectSignals();
    clearSecondMeasurePages();
    clearTripleMeasurePages();
    ui->frame->setVisible(false);
}

void MeasurementProcessUiHandler::initializeStateMachine()
{
    if(measureStateMachine)
        return;
    measureStateMachine = new MeasurementStateMachine(ui->measureDensityStage, this);

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::StartMeasure,
        ui->pageStartMeasure,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::InitialData,
        ui->pageInitialData,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::DryMeasure,
        ui->pageDryMeasure,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::PrepareSecond,
        ui->pagePrepareMeasureSecond,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::PrepareTriple,
        ui->pagePrepareMeasureTriple,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::FinishSecond,
        ui->pageFinishMeasurementSecond,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::SaturationMass,
        ui->pageSatruationMassTriple,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::FinishTriple,
        ui->pageFinishSaturatedMassTriple,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::SummarySecond,
        ui->pageSummarySecond,
        this
    });

    measureStateMachine->addStageConfiguration({
        MeasurementStages::Stage::SummaryTriple,
        ui->pageSummaryTriple,
        this
    });

    measureStateMachine->goToStage(MeasurementStages::Stage::StartMeasure);

    connect(measureStateMachine, &MeasurementStateMachine::stageChanged, this, &MeasurementProcessUiHandler::onStageChanged);
}

bool MeasurementProcessUiHandler::onEnterStage(MeasurementStages::Stage stage, MeasurementStages::Stage )
{
#ifdef DEBUG
    qDebug() << "Entering stage:" << static_cast<int>(stage) << "from:" << static_cast<int>(fromStage);
#endif
    switch(stage)
    {
    case MeasurementStages::Stage::StartMeasure:
        onStartMeasurePageEnter();
        break;

    case MeasurementStages::Stage::InitialData:
        onInitialDataPageEnter();
        break;

    case MeasurementStages::Stage::DryMeasure:
        onDryMeasurePageEnter();
        break;

    case MeasurementStages::Stage::PrepareSecond:
        onPrepareSecondPageEnter();
        break;

    case MeasurementStages::Stage::PrepareTriple:
        onPrepareTriplePageEnter();
        break;

    case MeasurementStages::Stage::FinishSecond:
        onFinishSecondPageEnter();
        break;

    case MeasurementStages::Stage::SaturationMass:
        onSaturationMassPageEnter();
        break;

    case MeasurementStages::Stage::FinishTriple:
        onFinishTriplePageEnter();
        break;

    case MeasurementStages::Stage::SummarySecond:
        onSummarySecondPageEnter();
        break;

    case MeasurementStages::Stage::SummaryTriple:
        onSummaryTriplePageEnter();
        break;

    default:
        break;
    }

    return true;
}

void MeasurementProcessUiHandler::onStartMeasurePageEnter()
{

}

void MeasurementProcessUiHandler::onInitialDataPageEnter()
{
    fillFluidCombo();
    fillSampleCombo();
    updateInitialDataLabels();

    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnableInitialDataPage(setStageResult);
}

void MeasurementProcessUiHandler::onDryMeasurePageEnter()
{
    updateDryDataLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnableDryMeasurePage(setStageResult);
}

void MeasurementProcessUiHandler::onPrepareSecondPageEnter()
{
    updatePrepareDataLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnablePrepareMeasureSecondPage(setStageResult);
}

void MeasurementProcessUiHandler::onPrepareTriplePageEnter()
{
    updatePrepareSaturationDataLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnablePrepareSaturationPage(setStageResult);
}

void MeasurementProcessUiHandler::onFinishSecondPageEnter()
{
    updateFinishMeasureSecondLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnableFinishMeasureSecondPage(setStageResult);
}

void MeasurementProcessUiHandler::onSaturationMassPageEnter()
{
    updateFinishMeasureTripleLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnableSaturationTrilpePage(setStageResult);
}

void MeasurementProcessUiHandler::onFinishTriplePageEnter()
{
    updateAirSaturatedTripleLabels();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    setEnableSaturatedTrilpePage(setStageResult);
}

void MeasurementProcessUiHandler::onSummarySecondPageEnter()
{
    updateMeasureSecondLabelsSummary();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    if(!setStageResult)
        return;
    radwagMeasureControler.setMeasureStatus(MeasurementStatus::Completed);
    radwagMeasureControler.endMeasure();
    clearSecondMeasurePages();
}

void MeasurementProcessUiHandler::onSummaryTriplePageEnter()
{
    updateMeasureTripleLabelsSummary();
    bool setStageResult = radwagMeasureControler.getStage() == measureStateMachine->getCurrentStage();
    if(!setStageResult)
        return;
    radwagMeasureControler.setMeasureStatus(MeasurementStatus::Completed);
    radwagMeasureControler.endMeasure();
    clearTripleMeasurePages();
}

bool MeasurementProcessUiHandler::onExitStage(MeasurementStages::Stage stage, MeasurementStages::Stage toStage)
{
    qDebug() << "Exiting stage:" << static_cast<int>(stage) << "to:" << static_cast<int>(toStage);

    bool canExit = false;

    if(toStage < stage)
        return true;

    switch(stage)
    {
    case MeasurementStages::Stage::StartMeasure:
        canExit = onStartMeasurePageDataExit();
        break;

    case MeasurementStages::Stage::InitialData:
        canExit = onInitialDataPageExit();
        break;

    case MeasurementStages::Stage::DryMeasure:
        canExit = onDryMeasureDataPageExit();
        break;

    case MeasurementStages::Stage::PrepareSecond:
        canExit = onPrepareSecondMeasurePageDataExit();
        break;

    case MeasurementStages::Stage::PrepareTriple:
        canExit = onPrepareTripleMeasurePageDataExit();
        break;

    case MeasurementStages::Stage::SaturationMass:
        canExit = onSaturationMassMeasurePageDataExit();
        break;

    case MeasurementStages::Stage::FinishSecond:
        canExit = onFinishSecondMeasurePageDataExit();
        break;

    case MeasurementStages::Stage::FinishTriple:
        canExit = onFinishTripleMeasurePageDataExit();
        break;

    default:
        return canExit;
    }

    if(canExit)
        radwagMeasureControler.setStage(toStage);

    return canExit;
}

void MeasurementProcessUiHandler::goToNextMeasureStage()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;
    measureStateMachine->goToNextStage();
}

void MeasurementProcessUiHandler::goToPreviousMeasureStage()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;
    measureStateMachine->goToPreviousStage();
}

void MeasurementProcessUiHandler::onStageChanged()
{
    updateStageLabels();
    ui->frame->setVisible(measureStateMachine->getCurrentStage() != MeasurementStages::Stage::StartMeasure);
    ui->groupBoxAdditionalSettings->setVisible(measureStateMachine->getCurrentStage() == MeasurementStages::Stage::InitialData);
}

bool MeasurementProcessUiHandler::onStartMeasurePageDataExit()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Brak połączenia", "Brak połączenia z urządzeniem");
        return false;
    }

    if(!checkGuidePrepareWorkstation())
        return false;

    if(!radwagMeasureControler.hasActiveMeasurement())
        radwagMeasureControler.beginNewMeasure();

    return true;
}

bool MeasurementProcessUiHandler::onInitialDataPageExit()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return false;

    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Brak połączenia", "Brak połączenia z urządzeniem");
        return false;
    }

    if(ui->comboBoxSampleSelection->currentText().isEmpty())
    {
        mainWindow->showWarning("Brak wybranej próbki", "Brak wybranej próbki");
        return false;
    }

    if(ui->editAuthor->text().isEmpty())
    {
        mainWindow->showWarning("Brak danych", "Pole autor nie może być puste");
        return false;
    }

    MeasurementType type = ui->radioMeasureSecond->isChecked() ? MeasurementType::TwoStage : MeasurementType::ThreeStage;
    auto sample = dataHolder.sampleManager->getSample(ui->comboBoxSampleSelection->currentData().toString());
    auto fluid = dataHolder.fluidManager->getFluid(ui->comboBoxFluid->currentText());
    QString author = ui->editAuthor->text().trimmed();
    radwagMeasureControler.setInitialData(type, sample, fluid, author);

    fillMeasuresTypeCombos();
    return true;
}

bool MeasurementProcessUiHandler::onDryMeasureDataPageExit()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Brak połączenia", "Brak połączenia z urządzeniem");
        return false;
    }

    if(ui->editSavedDryMeasure->text().isEmpty())
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }
    return true;
}

bool MeasurementProcessUiHandler::onPrepareSecondMeasurePageDataExit()
{
    if(!checkGuidePrepareMeasureSecondButton())
        return false;

    if(ui->comboBoxTempFluidPrepareMeasureSecond->currentIndex() == -1)
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz wybrać temperature cieczy");
        return false;
    }

    double selectedTemperature = ui->comboBoxTempFluidPrepareMeasureSecond->currentData().toDouble();
    radwagMeasureControler.setFluidTemperature(selectedTemperature);
    return true;
}

bool MeasurementProcessUiHandler::onFinishSecondMeasurePageDataExit()
{
    if(ui->editSavedValueFinishMeasureSecond->text().isEmpty())
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }
    return true;
}

bool MeasurementProcessUiHandler::onPrepareTripleMeasurePageDataExit()
{
    if(!checkGuideSampleSaturationPreparation())
        return false;

    if(ui->comboBoxTempFluidPrepareSaturationTrilpe->currentIndex() == -1)
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz wybrać temperature cieczy");
        return false;
    }

    if(ui->comboBoxSaturationMethodPrepareMeasureTriple->currentIndex() == -1)
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz wybrać metodę nasycania próbki");
        return false;
    }

    if(ui->spinSaturationTimePrepareMeasureTriple->value() == 0)
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz uzupełnić czas próbki");
        return false;
    }

    radwagMeasureControler.setSaturationMethod(static_cast<SaturationMethod>(ui->comboBoxSaturationMethodPrepareMeasureTriple->currentData().toInt()));
    radwagMeasureControler.setFluidTemperature(ui->comboBoxTempFluidPrepareSaturationTrilpe->currentData().toDouble());
    radwagMeasureControler.setSaturationTime(ui->spinSaturationTimePrepareMeasureTriple->value());
    radwagMeasureControler.setSaturationBeginDate(ui->editStartSaturationDateTimePrepareMeasureTriple->dateTime());
    return true;
}

bool MeasurementProcessUiHandler::onSaturationMassMeasurePageDataExit()
{
    if(ui->editSavedValueFinishMeasurementTriple->text().isEmpty())
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }
    return true;
}

bool MeasurementProcessUiHandler::onFinishTripleMeasurePageDataExit()
{
    if(ui->editSavedMeasureSaturated->text().isEmpty())
    {
        mainWindow->showWarning("Brak danych", "Aby przejść dalej musisz zapisać pomiar");
        return false;
    }
    return true;
}

void MeasurementProcessUiHandler::buttonTableFluidsOnClicked()
{
    QMap<QString, Fluid> fluids = dataHolder.fluidManager->getFluids();
    auto dialog = new FluidTablesDialog(fluids, mainWindow);
    dialog->exec();
    dataHolder.fluidManager->setFluids(fluids);
    fillFluidCombo();
}

void MeasurementProcessUiHandler::buttonSamplesOnClicked()
{
    QMap<QString, Sample> samples = dataHolder.sampleManager->getSamples();
    QMap<QString, Material> materials = dataHolder.materialManager->getMaterials();
    auto dialog = new SampleDialog(samples, materials, mainWindow);
    connect(dialog, &SampleDialog::materialsChanged, this, &MeasurementProcessUiHandler::onMaterialsChanged);
    dialog->exec();
    dataHolder.sampleManager->setSamples(samples);
    fillSampleCombo();
    upadteSampleEditors();
}

void MeasurementProcessUiHandler::onMaterialsChanged(const QMap<QString, Material> &materials)
{
    dataHolder.materialManager->setMaterials(materials);
}

void MeasurementProcessUiHandler::onShowHydroSetSchemeButtonClicked()
{
    QDialog *schemeDialog = new QDialog(mainWindow);
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

void MeasurementProcessUiHandler::onResetMeasureButtonClicked()
{
    clearSecondMeasurePages();
    clearTripleMeasurePages();
    measureStateMachine->goToStage(MeasurementStages::Stage::StartMeasure, true);
    ui->frame->setVisible(false);
    radwagMeasureControler.endMeasure(true);
}

void MeasurementProcessUiHandler::onSaveMeasureButtonClicekd()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    auto measure = radwagMeasureControler.getActiveMeasure();
    if(radwagMeasureControler.isContinued())
        dataHolder.measurementManager->removeMeasurement(measure->getId());

    if(dataHolder.measurementManager->addMeasurement(measure))
        mainWindow->showWarning(tr("Zapis pomiaru"), "Pomyślnie zapisano dane aktywnego pomiaru.");
}

void MeasurementProcessUiHandler::onMeasurementTypeChanged()
{
    if(ui->radioMeasureSecond->isChecked())
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageSecondMeasure);
    else
        ui->stackedWidgetStepsMeasure->setCurrentWidget(ui->stageTripleMeasure);

    measureStateMachine->setMeasurementType(!ui->radioMeasureSecond->isChecked());
    updateStageLabels();
}

void MeasurementProcessUiHandler::onSampleComboBoxChanged(int index)
{
    upadteSampleEditors();
}

void MeasurementProcessUiHandler::onDryMassExecuteStepOneClicked()
{
    if(!appCore.hasConnectionWithScale())
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");

    // appCore.radwagScaleConnector->sendZeroCommand();
    // appCore.radwagScaleConnector->sendTareCommand();

    auto scaleConnector = appCore.getScaleConnector();
    scaleConnector->sendZeroCommand();
    scaleConnector->sendTareCommand();
    ui->step1CheckBox->setChecked(true);
}

void MeasurementProcessUiHandler::onDryMassExecuteStepTwoClicked()
{
    if(!appCore.hasConnectionWithScale())
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");

    appCore.getScaleConnector()->sendImmediateWeightCommand();
    ui->step2CheckBox->setChecked(true);
}

void MeasurementProcessUiHandler::onDryMassExecuteStepThreeClicked()
{
    if(!ui->editCurrentDryMeasure->text().isEmpty())
    {
        onSaveCurrentDryMeasureButtonClicked();
        ui->step3CheckBox->setChecked(true);
    }
    else
        mainWindow->showWarning("Błąd", "Brak wartości do zapisania.");
}

void MeasurementProcessUiHandler::onGetCurrentDryMeasureButtonClicked()
{
    if(!appCore.hasConnectionWithScale())
        return;
    appCore.getScaleConnector()->sendImmediateWeightCommand();
}

void MeasurementProcessUiHandler::onSaveCurrentDryMeasureButtonClicked()
{
    QString text = ui->editCurrentDryMeasure->text().trimmed();
    if(!radwagMeasureControler.setDryMass(utils::getDouble(text)))
        mainWindow->showWarning(tr("Błąd"), "Błąd zapisu pomiaru!");
    ui->editSavedDryMeasure->setText(text);
}

void MeasurementProcessUiHandler::onClearSavedDryMeasureButtonClicked()
{
    ui->editSavedDryMeasure->clear();
    radwagMeasureControler.setDryMass(0.0);
}


void MeasurementProcessUiHandler::onFinishMeasureSecondStepTwoClicked()
{
    if(!appCore.hasConnectionWithScale())
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");

    appCore.getScaleConnector()->sendImmediateWeightCommand();
    ui->step2FinishMeasureSecondCheckBox->setChecked(true);
}

void MeasurementProcessUiHandler::onFinishMeasureSecondStepTreeClicked()
{
    if(!ui->editCurrentValueFinishSecond->text().isEmpty())
    {
        onSaveCurrentFinishMeasureSecondButtonClicked();
        ui->step3FinishMeasureSecondCheckBox->setChecked(true);
    }
    else
        mainWindow->showWarning("Błąd", "Brak wartości do zapisania.");
}

void MeasurementProcessUiHandler::onGetCurrentFinishMeasureSecondButtonClicked()
{
    if(!appCore.hasConnectionWithScale())
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");
    appCore.getScaleConnector()->sendImmediateWeightCommand();
}

void MeasurementProcessUiHandler::onSaveCurrentFinishMeasureSecondButtonClicked()
{
    QString text = ui->editCurrentValueFinishSecond->text().trimmed();
    if(!radwagMeasureControler.setMassInFluid(utils::getDouble(text)))
        mainWindow->showWarning(tr("Błąd"), "Błąd zapisu pomiaru!");
    ui->editSavedValueFinishMeasureSecond->setText(text);
    ui->editLiquidMeasureFinishMeasureSecond->setText(text + " g");
}

void MeasurementProcessUiHandler::onClearSavedFinishMeasureSecondButtonClicked()
{
    ui->editSavedValueFinishMeasureSecond->clear();
    ui->editLiquidMeasureFinishMeasureSecond->clear();
    radwagMeasureControler.setMassInFluid(0.0);
}

void MeasurementProcessUiHandler::onFinishMeasureTripleStepTwoClicked()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");
        return;
    }

    appCore.getScaleConnector()->sendImmediateWeightCommand();
    ui->step2FinishTriple->setChecked(true);
}

void MeasurementProcessUiHandler::onFinishMeasureTripleExecuteStepTreeClicked()
{
    if(!ui->editCurrentValueFinishMeasurementTriple->text().isEmpty())
    {
        onSaveTripleCurrentFinishMeasureButtonClicked();
        ui->step3FinishTriple->setChecked(true);
    }
    else
        mainWindow->showWarning("Błąd", "Brak wartości do zapisania.");
}

void MeasurementProcessUiHandler::onGetTripleCurrentFinishMeasureButtonClicked()
{
    if(!appCore.hasConnectionWithScale())
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");
    appCore.getScaleConnector()->sendImmediateWeightCommand();
}

void MeasurementProcessUiHandler::onSaveTripleCurrentFinishMeasureButtonClicked()
{
    QString text = ui->editCurrentValueFinishMeasurementTriple->text().trimmed();
    if(!radwagMeasureControler.setMassInFluid(utils::getDouble(text)))
        mainWindow->showWarning(tr("Błąd"), "Błąd zapisu pomiaru! FinishTriple");
    ui->editSavedValueFinishMeasurementTriple->setText(text);
    ui->editLiquidMeasureFinishMeasurementTriple->setText(text + " g");
}

void MeasurementProcessUiHandler::onClearTripleSavedFinishMeasureButtonClicked()
{
    ui->editSavedValueFinishMeasurementTriple->clear();
    ui->editLiquidMeasureFinishMeasurementTriple->clear();
    radwagMeasureControler.setMassInFluid(0.0);
}

void MeasurementProcessUiHandler::onSaturatedMassTripleStepFourClicked()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");
        return;
    }

    appCore.getScaleConnector()->sendImmediateWeightCommand();
    ui->step4CheckBoxSaturatedTriple->setChecked(true);
}

void MeasurementProcessUiHandler::onSaturatedMassTripleStepFiveClicked()
{
    if(!ui->editCurrentMeasureSaturated->text().isEmpty())
    {
        onSaveTripleCurrentSaturatedMeasureButtonClicked();
        ui->step5CheckBoxSaturatedTriple->setChecked(true);
    }
    else
        mainWindow->showWarning("Błąd", "Brak wartości do zapisania.");
}

void MeasurementProcessUiHandler::onGetTripleCurrentSaturatedMeasureButtonClicked()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->showWarning("Błąd", "Brak aktywnego połączenia z wagą.");
        return;
    }
    appCore.getScaleConnector()->sendImmediateWeightCommand();
}

void MeasurementProcessUiHandler::onSaveTripleCurrentSaturatedMeasureButtonClicked()
{
    QString text = ui->editCurrentMeasureSaturated->text().trimmed();
    if(!radwagMeasureControler.setSaturatedMass(utils::getDouble(text)))
        mainWindow->showWarning("Błąd", "Błąd zapisu pomiaru! SaturatedTriple");
    ui->editSavedMeasureSaturated->setText(text);
    ui->editAirSaturatedTriple->setText(text + " g");
}

void MeasurementProcessUiHandler::onClearTripleSavedSaturatedMeasureButtonClicked()
{
    ui->editSavedMeasureSaturated->clear();
    ui->editAirSaturatedTriple->clear();
    radwagMeasureControler.setSaturatedMass(0.0);
}

void MeasurementProcessUiHandler::onNewMeasureSummaryButtonClicked()
{
    radwagMeasureControler.beginNewMeasure();
    measureStateMachine->goToStage(checkGuidePrepareWorkstation() ? MeasurementStages::Stage::InitialData : MeasurementStages::Stage::StartMeasure, true);
    setEnableInitialDataPage(true);
}

void MeasurementProcessUiHandler::onReplyMeasureSummaryButtonClicked()
{
    measureStateMachine->goToStage(radwagMeasureControler.replyActiveMeasure() ? MeasurementStages::Stage::StartMeasure : MeasurementStages::Stage::DryMeasure, true);
}

void MeasurementProcessUiHandler::onSaveMeasureSummaryButtonClicked()
{
    dataHolder.measurementManager->saveMeasurements();
}

void MeasurementProcessUiHandler::fillSampleCombo()
{
    ui->comboBoxSampleSelection->clear();
    const QMap<QString, Sample> &samples = dataHolder.sampleManager->getSamples();
    for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
    {
        QString id = it.value().getId();
        QString name = it.value().getName();
        ui->comboBoxSampleSelection->addItem(it.value().getName(), it.value().getId());
    }
    ui->comboBoxSampleSelection->setCurrentIndex(-1);
}

void MeasurementProcessUiHandler::fillFluidCombo()
{
    ui->comboBoxFluid->clear();
    const QMap<QString, Fluid> &fluids = dataHolder.fluidManager->getFluids();
    for(auto &fluid : fluids)
        ui->comboBoxFluid->addItem(fluid.getName());
    ui->comboBoxFluid->setCurrentIndex(-1);
}

void MeasurementProcessUiHandler::fillSaturationMethodsCombo()
{
    ui->comboBoxSaturationMethodPrepareMeasureTriple->clear();
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::BoilingInWater), static_cast<int>(SaturationMethod::BoilingInWater));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::VacuumMethod), static_cast<int>(SaturationMethod::VacuumMethod));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->addItem(utils::getSaturationMethodName(SaturationMethod::LongTermSoaking), static_cast<int>(SaturationMethod::LongTermSoaking));
    ui->comboBoxSaturationMethodPrepareMeasureTriple->setCurrentIndex(-1);

    ui->spinSaturationTimePrepareMeasureTriple->setValue(0);
}

void MeasurementProcessUiHandler::fillMeasuresTypeCombos()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;
    if(radwagMeasureControler.getActiveMeasure()->isThreeType())
    {
        fillSaturationMethodsCombo();
        fillPrepareSaturationTemperatureCombo();
    }
    else
        fillPrepareMeasureTemperatureCombo();
}

void MeasurementProcessUiHandler::fillPrepareMeasureTemperatureCombo()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    ui->comboBoxTempFluidPrepareMeasureSecond->clear();
    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

    for(auto it = densityTableMap.keyBegin(); it != densityTableMap.keyEnd(); it++)
        ui->comboBoxTempFluidPrepareMeasureSecond->addItem(QString::number(*it) + " °C", *it);
}

void MeasurementProcessUiHandler::fillPrepareSaturationTemperatureCombo()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    ui->comboBoxTempFluidPrepareSaturationTrilpe->clear();
    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

    for(auto it = densityTableMap.keyBegin(); it != densityTableMap.keyEnd(); it++)
        ui->comboBoxTempFluidPrepareSaturationTrilpe->addItem(QString::number(*it) + " °C", *it);
}

void MeasurementProcessUiHandler::upadteSampleEditors()
{
    clearSampleEditors();

    QString smapleId = ui->comboBoxSampleSelection->currentData().toString();
    Sample sample = dataHolder.sampleManager->getSample(smapleId);
    if(sample.getId().isEmpty())
        return;

    ui->editSampleId->setText(smapleId);
    ui->editSampleName->setText(sample.getName());
    ui->editMaterial->setText(sample.getMaterialName());
    ui->editMaterialDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 4) +  " g/cm³");
    ui->editSampleDescription->setPlainText(sample.getDescription());
}


bool MeasurementProcessUiHandler::updateInitialDataLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return false;

    auto activeMeasure = radwagMeasureControler.getActiveMeasure();
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

void MeasurementProcessUiHandler::updateDryDataLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    QString value = QString::number(radwagMeasureControler.getActiveMeasure()->getSampleDryMass(), 'f', 3);
    ui->editCurrentDryMeasure->setText(value);
    ui->editSavedDryMeasure->setText(value);
}

void MeasurementProcessUiHandler::updatePrepareDataLabels()
{
    updateSampleInfoLabels();
    updateFluidInfoLabels();
    updateTemperatureComboBox();
    updateFluidDensityLabel();
}

void MeasurementProcessUiHandler::updateSampleInfoLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler.getActiveMeasure()->getSample();

    ui->editSampleIdValuePrepareMeasureSecond->setText(currentSample.getId());
    ui->editSampleMaterialValuePrepareMeasureSecond->setText(currentSample.getMaterialName());

    double airMass = radwagMeasureControler.getDryMass();
    QString formattedAirMass = QString::number(airMass, 'f', 3) + " g";
    ui->labelAirWeightValuePrepareMeasureSecond->setText(formattedAirMass);
}

void MeasurementProcessUiHandler::updateFluidInfoLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    ui->editLiquidTypePrepareMeasureSecond->setText(currentFluid.getName());
}

void MeasurementProcessUiHandler::updateTemperatureComboBox()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    const QMap<double, double> &densityTableMap = currentFluid.getDensityTableMap();

    double currentTemperature = radwagMeasureControler.getFluidTemperature();
    int temperatureIndex = ui->comboBoxTempFluidPrepareMeasureSecond->findData(currentTemperature);

    if(temperatureIndex != -1)
    {
        ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(temperatureIndex);
        const QMap<double, double> &densityMap = currentFluid.getDensityTableMap();
        if(densityMap.contains(currentTemperature))
        {
            double density = densityMap.value(currentTemperature);
            ui->labelLiquidDensityValuePrepareMeasureSecond->setText(QString::number(density, 'f', 1) + " g/cm³");
        }
    }
    else
    {
        ui->comboBoxTempFluidPrepareMeasureSecond->setCurrentIndex(-1);
        ui->labelLiquidDensityValuePrepareMeasureSecond->setText(QString("0.0 g/cm³"));
    }
}

void MeasurementProcessUiHandler::updateFinishMeasureSecondLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler.getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();

    ui->editSampleIdValueFinishMeasureSecond->setText(currentSample.getId());
    ui->editSampleMaterialValueFinishMeasureSecond->setText(currentSample.getMaterialName());
    ui->editLiquidFinishMeasureSecond->setText(currentFluid.getName());

    double airMass = radwagMeasureControler.getDryMass();
    double fluidDensity = radwagMeasureControler.getFluidDensity();

    ui->editDryMeasureFinishMeasureSecond->setText(QString::number(airMass, 'f', 3) + " g");
    ui->editDensityLiquidFinishMeasureSecond->setText(QString::number(fluidDensity, 'f', 5) + " g/cm³");
    ui->editSavedValueFinishMeasureSecond->setText(QString::number(radwagMeasureControler.getActiveMeasure()->getSampleInFluidMass(), 'f', 4) + " g");
}

void MeasurementProcessUiHandler::updateMeasureSecondLabelsSummary()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    auto measurement = radwagMeasureControler.getActiveMeasure();
    auto results = radwagMeasureControler.calculateResults();

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

void MeasurementProcessUiHandler::updatePrepareSaturationDataLabels()
{
    updateSaturationMethodPrepareTriple();
    updateSaturationSampleInfoLabels();
    updatePrepareSaturationFluidInfoLabels();
}

void MeasurementProcessUiHandler::updateSaturationSampleInfoLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler.getActiveMeasure()->getSample();
    ui->ediSampleIdValuePrepareMeasureTriple->setText(currentSample.getId());
    ui->editSampleMaterialValuePrepareMeasureTriple->setText(currentSample.getMaterialName());

    double airMass = radwagMeasureControler.getDryMass();
    QString formattedAirMass = QString::number(airMass, 'f', 3) + " g";
    ui->editAirWeightValuePrepareMeasureTriple->setText(formattedAirMass);

    ui->editStartSaturationDateTimePrepareMeasureTriple->setMinimumDate(QDate::currentDate());//
    ui->editStartSaturationDateTimePrepareMeasureTriple->setDateTime(radwagMeasureControler.getSaturationBeginDate());
}

void MeasurementProcessUiHandler::updatePrepareSaturationFluidInfoLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    ui->editLiquidTypePrepareSaturationTriple->setText(currentFluid.getName());
    updatePrepareSaturationTemperatureCombo();

}

void MeasurementProcessUiHandler::updatePrepareSaturationTemperatureCombo()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    double currentTemperature = radwagMeasureControler.getFluidTemperature();
    int temperatureIndex = ui->comboBoxTempFluidPrepareSaturationTrilpe->findData(currentTemperature);

    if(temperatureIndex != -1)
    {
        ui->comboBoxTempFluidPrepareSaturationTrilpe->setCurrentIndex(temperatureIndex);
        const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
        const QMap<double, double> &densityMap = currentFluid.getDensityTableMap();
        if(densityMap.contains(currentTemperature))
        {
            double density = densityMap.value(currentTemperature);
            ui->labelFluidDensityPrepareSaturationTrilpe->setText(QString::number(density, 'f', 5) + " g/cm³");
        }
    }
    else
    {
        ui->comboBoxTempFluidPrepareSaturationTrilpe->setCurrentIndex(-1);
        ui->labelFluidDensityPrepareSaturationTrilpe->setText(QString("0.0 g/cm³"));
    }
}

void MeasurementProcessUiHandler::updateFinishMeasureTripleLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler.getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();

    double airMass = radwagMeasureControler.getDryMass();
    double fluidDensity = radwagMeasureControler.getFluidDensity();

    ui->editSampleIdValueSaturationTriple->setText(currentSample.getId());
    ui->editMaterialNameSaturationTriple->setText(currentSample.getMaterialName());
    ui->editFluidNameSaturationTriple->setText(currentFluid.getName());
    ui->editSaturationMethodSaturationTriple->setText(utils::getSaturationMethodName(static_cast<SaturationMethod>(radwagMeasureControler.getActiveMeasure()->getSaturationMethod())));
    ui->editFluidDensitySaturationTriple->setText(QString::number(fluidDensity, 'f', 5) + " g");
    ui->editDryMassSaturationTriple->setText(QString::number(airMass, 'f', 3) + " g");
    ui->editSavedValueFinishMeasurementTriple->setText(QString::number(radwagMeasureControler.getActiveMeasure()->getSampleInFluidMass(), 'f', 4) + " g");
}

void MeasurementProcessUiHandler::updateAirSaturatedTripleLabels()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    const Sample &currentSample = radwagMeasureControler.getActiveMeasure()->getSample();
    const Fluid &currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();

    ui->editSampleIdAirSaturatedTriple->setText(currentSample.getId());
    ui->editMaterialAirSaturatedTriple->setText(currentSample.getMaterialName());
    ui->editFluidAirSaturatedTriple->setText(currentFluid.getName());
    ui->editSaturationAirSaturatedTriple->setText(utils::getSaturationMethodName(static_cast<SaturationMethod>(radwagMeasureControler.getActiveMeasure()->getSaturationMethod())));

    double airMass = radwagMeasureControler.getDryMass();
    ui->editDryAirSaturatedTriple->setText(QString::number(airMass, 'f', 3) + " g");

    double fluidDensity = radwagMeasureControler.getFluidDensity();
    ui->editDensityAirSaturatedTriple->setText(QString::number(fluidDensity, 'f', 5) + " g/cm³");

    double saturationMassInFluid = radwagMeasureControler.getMassInFluid();
    ui->editLiquidAirSaturatedTriple->setText(QString::number(saturationMassInFluid, 'f', 3) + " g");
    ui->editSavedMeasureSaturated->setText(QString::number(radwagMeasureControler.getActiveMeasure()->getSampleSaturatedMass(), 'f', 4) + " g");
}

void MeasurementProcessUiHandler::updateMeasureTripleLabelsSummary()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    auto measurement = radwagMeasureControler.getActiveMeasure();
    auto results = radwagMeasureControler.calculateResults();
    auto sample = measurement->getSample();
    Fluid fluid = measurement->getFluid();

    ui->valueTripleMeasureID->setText(measurement->getSampleId());
    ui->valueTripleSampleName->setText(sample.getName());
    ui->valueTripleMaterialName->setText(sample.getMaterialName());
    ui->valueTripleTheoreticalDensity->setText(QString::number(sample.getMaterialDensity(), 'f', 4) + " g/cm³");

    ui->valueTripleMeasureLiquidName->setText(fluid.getName());
    ui->valueTripleMeasureLiquidDensity->setText(QString::number(measurement->getFluidDensity(), 'f', 5) + " g/cm³");
    ui->valueTripleMeasureSaturationMethod->setText(utils::getSaturationMethodName(measurement->getSaturationMethod()));
    ui->valueSaturationBeginDate->setText(radwagMeasureControler.getSaturationBeginDate().toString("dd-MM-yyyy hh:mm:ss"));
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

    updatePorosityChart();
    updateDenistyChart();
}

void MeasurementProcessUiHandler::updateSaveCurrentDryMeasureButtonState()
{
    ui->buttonSaveCurrentDryMeasure->setEnabled(utils::getDouble(ui->editCurrentDryMeasure->text()) > 0.0);
}

void MeasurementProcessUiHandler::updateSaveFinishSecondButtonState()
{
    ui->buttonSaveCurrentFinishSecond->setEnabled(utils::getDouble(ui->editCurrentValueFinishSecond->text()) > 0.0);
}

void MeasurementProcessUiHandler::updateSaveFinishTripleButtonState()
{
    ui->buttonSaveMeasureFinishMeasurementTriple->setEnabled(utils::getDouble(ui->editCurrentValueFinishMeasurementTriple->text()) > 0.0);
}

void MeasurementProcessUiHandler::updateSaveSaturatedTripleButtonState()
{
    ui->buttonSaveCurrentMeasureSaturated->setEnabled(utils::getDouble(ui->editCurrentMeasureSaturated->text()) > 0.0);
}

void MeasurementProcessUiHandler::updateFluidDensityLabel()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    Fluid currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    const QMap<double, double>& densityTableMap = currentFluid.getDensityTableMap();

    int currentIndex = ui->comboBoxTempFluidPrepareMeasureSecond->currentIndex();
    double selectedTemperature = ui->comboBoxTempFluidPrepareMeasureSecond->currentData().toDouble();
    if(densityTableMap.contains(selectedTemperature))
    {
        double density = densityTableMap.value(selectedTemperature);
        QString formattedDensity = QString::number(density, 'f', 5) + " g/cm³";
        ui->labelLiquidDensityValuePrepareMeasureSecond->setText(formattedDensity);
    }
    else
       ui->labelLiquidDensityValuePrepareMeasureSecond->setText("0.0 g/cm³");
}


void MeasurementProcessUiHandler::updateSaturationMethodPrepareTriple()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;
    SaturationMethod saturationMethod = radwagMeasureControler.getActiveMeasure()->getSaturationMethod();
    int index = ui->comboBoxSaturationMethodPrepareMeasureTriple->findData(static_cast<int>(saturationMethod));
    if(index != -1)
        ui->comboBoxSaturationMethodPrepareMeasureTriple->setCurrentIndex(index);
}

void MeasurementProcessUiHandler::updatePrepareSaturationFluidDensityLabel()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;

    int currentIndex = ui->comboBoxTempFluidPrepareSaturationTrilpe->currentIndex();
    if(currentIndex == -1)
        return;

    double selectedTemperature = ui->comboBoxTempFluidPrepareSaturationTrilpe->itemData(currentIndex).toDouble();
    Fluid currentFluid = radwagMeasureControler.getActiveMeasure()->getFluid();
    const QMap<double, double>& densityTableMap = currentFluid.getDensityTableMap();
    if(!densityTableMap.contains(selectedTemperature))
        return;

    double density = densityTableMap.value(selectedTemperature);
    QString formattedDensity = QString::number(density, 'f', 5) + " g/cm³";
    ui->labelFluidDensityPrepareSaturationTrilpe->setText(formattedDensity);
}


void MeasurementProcessUiHandler::setupCharts()
{
    if(ui->framePorosityChart)
    {
        porosityChart = new PorosityChartWidget(mainWindow);
        porosityChart->setChartType(BaseChartType::PieChart);

        auto layout = new QVBoxLayout(ui->framePorosityChart);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(porosityChart);
    }
    if(ui->frameDensityChart)
    {
        denistyChart = new DensityChartWidget(mainWindow);
        denistyChart->setChartType(BaseChartType::BarChart);

        auto layoutDensity = new QVBoxLayout(ui->frameDensityChart);
        layoutDensity->addWidget(denistyChart);
    }
}

void MeasurementProcessUiHandler::updatePorosityChart()
{
    if(!porosityChart)
        return;

    auto activeMeasure = radwagMeasureControler.getActiveMeasure();
    if(activeMeasure)
    {
        auto results = dataHolder.measurementManager->getResults(activeMeasure->getId());
        porosityChart->updateChart(results);
    }
    else
        porosityChart->clearChart();
}

void MeasurementProcessUiHandler::updateDenistyChart()
{
    if(!denistyChart)
        return;

    auto activeMeasure = radwagMeasureControler.getActiveMeasure();
    if(activeMeasure)
    {
        auto results = dataHolder.measurementManager->getResults(activeMeasure->getId());
        denistyChart->updateChart(results);
    }
    else
        denistyChart->clearChart();
}

bool MeasurementProcessUiHandler::checkGuidePrepareWorkstation(bool showMessage)
{
    if(!ui->checkBoxStep1PrepareWorkstation->isChecked() ||
        !ui->checkBoxStep2PrepareWorkstation->isChecked() ||
        !ui->checkBoxStep3PrepareWorkstation->isChecked() ||
        !ui->checkBoxStep4PrepareWorkstation->isChecked() ||
        !ui->checkBoxStep5PrepareWorkstation->isChecked())
    {
        if(showMessage)
            mainWindow->showWarning("Niepełne przygotowanie", "Przed rozpoczęciem pomiaru wykonaj wszystkie kroki przygotowawcze.");
        return false;
    }
    return true;
}

bool MeasurementProcessUiHandler::checkGuidePrepareMeasureSecondButton()
{
    if(!ui->stepFirstPrepareMeasureSecond->isChecked() ||
        !ui->stepTwoPrepareMeasureSecond->isChecked() ||
        !ui->stepTreePrepareMeasureSecond->isChecked() ||
        !ui->stepFourPrepareMeasureSecond->isChecked() ||
        !ui->stepFivePrepareMeasureSecond->isChecked())
    {
        mainWindow->showWarning("Niepełne przygotowanie", "Przed rozpoczęciem pomiaru wykonaj wszystkie kroki przygotowawcze (część druga).");
        return false;
    }
    return true;
}

bool MeasurementProcessUiHandler::checkGuideSampleSaturationPreparation()
{
    if(!ui->step1CheckBoxPrepareSaturation->isChecked() ||
        !ui->step2CheckBoxPrepareSaturation->isChecked() ||
        !ui->step3CheckBoxPrepareSaturation->isChecked() ||
        !ui->step4CheckBoxPrepareSaturation->isChecked() ||
        !ui->step5CheckBoxPrepareSaturation->isChecked() ||
        !ui->step6CheckBoxPrepareSaturation->isChecked())
    {
        mainWindow->showWarning("Niepełne przygotowanie", "Przed kontynuacją pomiaru wykonaj wszystkie wymagane kroki.");
        return false;
    }
    return true;
}

void MeasurementProcessUiHandler::clearSecondMeasurePages()
{
    clearInitialDataPage();
    clearDryMeasurePage();
    clearPrepareMeasureSecondPage();
    clearFinishMeasureSecondPage();
}

void MeasurementProcessUiHandler::clearTripleMeasurePages()
{
    clearInitialDataPage();
    clearDryMeasurePage();
    clearPrepareMeasureTriplePage();
    clearFinishMeasureTriplePage();
    clearAirSaturatedTriplePage();
}

void MeasurementProcessUiHandler::clearSampleEditors()
{
    ui->editSampleId->clear();
    ui->editSampleName->clear();
    ui->editMaterial->clear();
    ui->editMaterialDensity->clear();
    ui->editSampleDescription->clear();
}

void MeasurementProcessUiHandler::clearInitialDataPage()
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

void MeasurementProcessUiHandler::clearDryMeasurePage()
{
    // ui->step1CheckBox->setChecked(false);
    // ui->step2CheckBox->setChecked(false);
    // ui->step3CheckBox->setChecked(false);

    ui->editCurrentDryMeasure->clear();
    ui->editSavedDryMeasure->clear();
}

void MeasurementProcessUiHandler::clearPrepareMeasureSecondPage()
{
    // if(radwagMeasureControler.isContinued())
    // {
    //     ui->stepFirstPrepareMeasureSecond->setChecked(false);
    //     ui->stepTwoPrepareMeasureSecond->setChecked(false);
    //     ui->stepTreePrepareMeasureSecond->setChecked(false);
    //     ui->stepFourPrepareMeasureSecond->setChecked(false);
    //     ui->stepFivePrepareMeasureSecond->setChecked(false);
    // }

    ui->comboBoxTempFluidPrepareMeasureSecond->clear();
    ui->editSampleIdValuePrepareMeasureSecond->clear();
    ui->editLiquidTypePrepareMeasureSecond->clear();
    ui->editSampleMaterialValuePrepareMeasureSecond->clear();
    ui->labelLiquidDensityValuePrepareMeasureSecond->clear();
    ui->labelAirWeightValuePrepareMeasureSecond->clear();
}

void MeasurementProcessUiHandler::clearFinishMeasureSecondPage()
{
    // ui->step3CheckBox_6->setChecked(false);
    // ui->step2FinishMeasureSecondCheckBox->setChecked(false);
    // ui->step3FinishMeasureSecondCheckBox->setChecked(false);

    ui->editCurrentValueFinishSecond->clear();
    ui->editSavedValueFinishMeasureSecond->clear();
    ui->editDryMeasureFinishMeasureSecond->clear();
    ui->editLiquidMeasureFinishMeasureSecond->clear();
    ui->editSampleIdValueFinishMeasureSecond->clear();
    ui->editSampleMaterialValueFinishMeasureSecond->clear();
    ui->editLiquidFinishMeasureSecond->clear();
    ui->editDensityLiquidFinishMeasureSecond->clear();
}

void MeasurementProcessUiHandler::clearPrepareMeasureTriplePage()
{
    // if(!radwagMeasureControler.isContinued())
    // {
    //     ui->step1CheckBoxPrepareSaturation->setChecked(false);
    //     ui->step2CheckBoxPrepareSaturation->setChecked(false);
    //     ui->step3CheckBoxPrepareSaturation->setChecked(false);
    //     ui->step4CheckBoxPrepareSaturation->setChecked(false);
    //     ui->step5CheckBoxPrepareSaturation->setChecked(false);
    //     ui->step6CheckBoxPrepareSaturation->setChecked(false);
    // }

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

void MeasurementProcessUiHandler::clearFinishMeasureTriplePage()
{
    // ui->step1FinishTriple->setChecked(false);
    // ui->step2FinishTriple->setChecked(false);
    // ui->step3FinishTriple->setChecked(false);

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

void MeasurementProcessUiHandler::clearAirSaturatedTriplePage()
{
    // ui->step1CheckBox_2->setChecked(false);
    // ui->step2CheckBox_2->setChecked(false);
    // ui->step3CheckBox_2->setChecked(false);
    // ui->step4CheckBoxSaturatedTriple->setChecked(false);
    // ui->step5CheckBoxSaturatedTriple->setChecked(false);

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

void MeasurementProcessUiHandler::setEnableInitialDataPage(bool enabled)
{
    ui->radioMeasureSecond->setEnabled(enabled);
    ui->radioMeasureTriple->setEnabled(enabled);
    ui->comboBoxSampleSelection->setEnabled(enabled);
    ui->editAuthor->setEnabled(enabled);
    ui->comboBoxFluid->setEnabled(enabled);
    ui->buttonSamples->setEnabled(enabled);
    ui->buttonTableFluids->setEnabled(enabled);
}

void MeasurementProcessUiHandler::setEnableDryMeasurePage(bool enabled)
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

void MeasurementProcessUiHandler::setEnablePrepareMeasureSecondPage(bool enabled)
{

    ui->stepFirstPrepareMeasureSecond->setEnabled(enabled);
    ui->stepTwoPrepareMeasureSecond->setEnabled(enabled);
    ui->stepTreePrepareMeasureSecond->setEnabled(enabled);
    ui->stepFourPrepareMeasureSecond->setEnabled(enabled);
    ui->stepFivePrepareMeasureSecond->setEnabled(enabled);
    ui->buttonConfrimPrepareMeasureSecond->setEnabled(enabled);
    ui->comboBoxTempFluidPrepareMeasureSecond->setEnabled(enabled);
}

void MeasurementProcessUiHandler::setEnableFinishMeasureSecondPage(bool enabled)
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

void MeasurementProcessUiHandler::setEnablePrepareSaturationPage(bool enabled)
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

void MeasurementProcessUiHandler::setEnableSaturationTrilpePage(bool enabled)
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

void MeasurementProcessUiHandler::setEnableSaturatedTrilpePage(bool enabled)
{
    ui->step1CheckBox_2->setEnabled(enabled);
    ui->step2CheckBox_2->setEnabled(enabled);
    ui->step3CheckBox_2->setEnabled(enabled);
    ui->step4CheckBoxSaturatedTriple->setEnabled(enabled);
    ui->step5CheckBoxSaturatedTriple->setEnabled(enabled);

    ui->buttonSaturatedMassExecuteStepFour->setEnabled(enabled);
    ui->buttonSaturatedMassExecuteStepFive->setEnabled(enabled);

    ui->buttonGetMeasureSaturated->setEnabled(enabled);
    ui->buttonSaveCurrentMeasureSaturated->setEnabled(enabled);
    ui->buttonClearSavedMeasureSaturated->setEnabled(enabled);
}

void MeasurementProcessUiHandler::onConfrimPrepareWorkstationButtonClicked()
{
    ui->checkBoxStep1PrepareWorkstation->setChecked(true);
    ui->checkBoxStep2PrepareWorkstation->setChecked(true);
    ui->checkBoxStep3PrepareWorkstation->setChecked(true);
    ui->checkBoxStep4PrepareWorkstation->setChecked(true);
    ui->checkBoxStep5PrepareWorkstation->setChecked(true);
}

void MeasurementProcessUiHandler::onConfrimPrepareMeasureSecondButtonClicked()
{
    ui->stepFirstPrepareMeasureSecond->setChecked(true);
    ui->stepTwoPrepareMeasureSecond->setChecked(true);
    ui->stepTreePrepareMeasureSecond->setChecked(true);
    ui->stepFourPrepareMeasureSecond->setChecked(true);
    ui->stepFivePrepareMeasureSecond->setChecked(true);
}

void MeasurementProcessUiHandler::onConfirmSampleSaturationPreparationClicked()
{
    ui->step1CheckBoxPrepareSaturation->setChecked(true);
    ui->step2CheckBoxPrepareSaturation->setChecked(true);
    ui->step3CheckBoxPrepareSaturation->setChecked(true);
    ui->step4CheckBoxPrepareSaturation->setChecked(true);
    ui->step5CheckBoxPrepareSaturation->setChecked(true);
    ui->step6CheckBoxPrepareSaturation->setChecked(true);
}

void MeasurementProcessUiHandler::onSpinSaturationTimeChanged()
{
    if(!radwagMeasureControler.hasActiveMeasurement())
        return;
    radwagMeasureControler.setSaturationTime(ui->spinSaturationTimePrepareMeasureTriple->value());
}

void MeasurementProcessUiHandler::onNewMeasure()
{
    if(radwagMeasureControler.hasActiveMeasurement())
    {
        mainWindow->showWarning("Aktywny pomiar", "Masz aktywny pomiar hydrostatyczny. Zakończ aktualny pomiar aby wykonać kolejny.");
        return;
    }

    ui->actionLibraryNewMeasure->trigger();
    measureStateMachine->goToStage(checkGuidePrepareWorkstation(false) ? MeasurementStages::Stage::InitialData : MeasurementStages::Stage::StartMeasure);
}

void MeasurementProcessUiHandler::onContinueMeasure(const std::shared_ptr<const Measurement> &sourceMeasure)
{
    ui->actionLibraryContinueMeasure->trigger();

    if(radwagMeasureControler.hasActiveMeasurement())
    {
        mainWindow->showWarning("Aktywny pomiar", "Masz aktywny pomiar hydrostatyczny. Zakończ aktualny pomiar aby wykonać kolejny.");
        return;
    }

    if(!appCore.hasConnectionWithScale() || !radwagMeasureControler.continueMeasure(sourceMeasure))
        return;
    measureStateMachine->goToStage(checkGuidePrepareWorkstation(false) ? radwagMeasureControler.getStage() : MeasurementStages::Stage::StartMeasure, true);
}

void MeasurementProcessUiHandler::connectSignals()
{
    connectNavMeasurementButtons();
    connectStagesOperationButtons();
    connectPrepareWorksationPageButtons();
    connectInitialDataPageButtons();
    connectPrepareMeasureSecondPageButtons();
    connectPrepareSaturationButton();
    connectDryMassPageButtons();
    connectFinishSecondPageButtons();
    connectFinishTriplePageButtons();
    connectSaturatedTriplePageButtons();
    connectSummaryMeasureSecondPageButtons();
    connectSummaryMeasureTriplePageButtons();
    connectCatalogsButtons();
}

void MeasurementProcessUiHandler::connectNavMeasurementButtons()
{
    connect(ui->buttonPrevData, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
    connect(ui->buttonNextData, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);

    connect(ui->buttonNextDryMass, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevDryMass, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
    connect(ui->buttonNextPreparation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevPreparation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
    connect(ui->buttonFinishMeasurementSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevFluidMass, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);

    connect(ui->buttonNextPrepareSaturation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevPrepareSaturation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
    connect(ui->buttonNextSaturation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevSaturation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
    connect(ui->buttonFinishMeasurementTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToNextMeasureStage);
    connect(ui->buttonPrevSaturatedMass, &QPushButton::clicked, this, &MeasurementProcessUiHandler::goToPreviousMeasureStage);
}

void MeasurementProcessUiHandler::connectStagesOperationButtons()
{
    connect(ui->buttonResetMeasureStagesSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onResetMeasureButtonClicked);
    connect(ui->buttonSaveMeasureToLibraryStagesSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveMeasureButtonClicekd);
    connect(ui->buttonResetMeasureStagesTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onResetMeasureButtonClicked);
    connect(ui->buttonSaveMeasureToLibraryStagesTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveMeasureButtonClicekd);
}

void MeasurementProcessUiHandler::connectPrepareWorksationPageButtons()
{
    connect(ui->buttonShowHydroSetScheme, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onShowHydroSetSchemeButtonClicked);
    connect(ui->buttonConfrimPrepareWorkstation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onConfrimPrepareWorkstationButtonClicked);
    connect(ui->buttonStartMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onStartMeasureButtonClicked);
}

void MeasurementProcessUiHandler::connectInitialDataPageButtons()
{
    connect(ui->radioMeasureSecond, &QRadioButton::toggled, this, &MeasurementProcessUiHandler::onMeasurementTypeChanged);
    connect(ui->radioMeasureTriple, &QRadioButton::toggled, this, &MeasurementProcessUiHandler::onMeasurementTypeChanged);
    connect(ui->comboBoxSampleSelection, &QComboBox::currentIndexChanged, this, &MeasurementProcessUiHandler::onSampleComboBoxChanged);
}

void MeasurementProcessUiHandler::connectPrepareMeasureSecondPageButtons()
{
    connect(ui->buttonConfrimPrepareMeasureSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onConfrimPrepareMeasureSecondButtonClicked);
    connect(ui->comboBoxTempFluidPrepareMeasureSecond, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementProcessUiHandler::updateFluidDensityLabel);
}

void MeasurementProcessUiHandler::connectPrepareSaturationButton()
{
    connect(ui->buttonConfirmPrepareSaturation, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onConfirmSampleSaturationPreparationClicked);
    connect(ui->comboBoxTempFluidPrepareSaturationTrilpe, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementProcessUiHandler::updatePrepareSaturationFluidDensityLabel);
    connect(ui->spinSaturationTimePrepareMeasureTriple, &QSpinBox::valueChanged, this, &MeasurementProcessUiHandler::onSpinSaturationTimeChanged);
}

void MeasurementProcessUiHandler::connectDryMassPageButtons()
{
    connect(ui->buttonDryMassExecuteStepOne, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onDryMassExecuteStepOneClicked);
    connect(ui->buttonDryMassExecuteStepTwo, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onDryMassExecuteStepTwoClicked);
    connect(ui->buttonDryMassExecuteStepThree, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onDryMassExecuteStepThreeClicked);

    connect(ui->buttonGetCurrentDryMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onGetCurrentDryMeasureButtonClicked);
    connect(ui->buttonSaveCurrentDryMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveCurrentDryMeasureButtonClicked);
    connect(ui->buttonClearEditSavedDryMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onClearSavedDryMeasureButtonClicked);

    connect(ui->editCurrentDryMeasure, &QLineEdit::textChanged, this, &MeasurementProcessUiHandler::updateSaveCurrentDryMeasureButtonState);
}

void MeasurementProcessUiHandler::connectFinishSecondPageButtons()
{
    connect(ui->buttonFinishMeasureSecondExecuteStepTwo, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onFinishMeasureSecondStepTwoClicked);
    connect(ui->buttonFinishMeasureSecondExecuteStepTree, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onFinishMeasureSecondStepTreeClicked);

    connect(ui->buttonGetCurrentValueFinishMeasureSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onGetCurrentFinishMeasureSecondButtonClicked);
    connect(ui->buttonSaveCurrentFinishSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveCurrentFinishMeasureSecondButtonClicked);
    connect(ui->buttonClearSavedValueFinishMeasureSecond, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onClearSavedFinishMeasureSecondButtonClicked);

    connect(ui->editCurrentValueFinishSecond, &QLineEdit::textChanged, this, &MeasurementProcessUiHandler::updateSaveFinishSecondButtonState);
}

void MeasurementProcessUiHandler::connectFinishTriplePageButtons()
{
    connect(ui->buttonFinishMeasureTripleExecuteStepTwo, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onFinishMeasureTripleStepTwoClicked);
    connect(ui->buttonFinishMeasureTripleExecuteStepThree, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onFinishMeasureTripleExecuteStepTreeClicked);

    connect(ui->buttonGetCurrentMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onGetTripleCurrentFinishMeasureButtonClicked);
    connect(ui->buttonSaveMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveTripleCurrentFinishMeasureButtonClicked);
    connect(ui->buttonClearSavedMeasureFinishMeasurementTriple, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onClearTripleSavedFinishMeasureButtonClicked);

    connect(ui->editCurrentValueFinishMeasurementTriple, &QLineEdit::textChanged, this, &MeasurementProcessUiHandler::updateSaveFinishSecondButtonState);
}

void MeasurementProcessUiHandler::connectSaturatedTriplePageButtons()
{
    connect(ui->buttonSaturatedMassExecuteStepFour, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaturatedMassTripleStepFourClicked);
    connect(ui->buttonSaturatedMassExecuteStepFive, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaturatedMassTripleStepFiveClicked);

    connect(ui->buttonGetMeasureSaturated, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onGetTripleCurrentSaturatedMeasureButtonClicked);
    connect(ui->buttonSaveCurrentMeasureSaturated, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveTripleCurrentSaturatedMeasureButtonClicked);
    connect(ui->buttonClearSavedMeasureSaturated, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onClearTripleSavedSaturatedMeasureButtonClicked);

    connect(ui->editCurrentMeasureSaturated, &QLineEdit::textChanged, this, &MeasurementProcessUiHandler::updateSaveSaturatedTripleButtonState);
}

void MeasurementProcessUiHandler::connectSummaryMeasureSecondPageButtons()
{
    connect(ui->buttonNewSecondMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onNewMeasureSummaryButtonClicked);
    connect(ui->buttonReplySecondMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onReplyMeasureSummaryButtonClicked);
    connect(ui->buttonSaveSecondMeasureToLibrary, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onSaveMeasureSummaryButtonClicked);
}

void MeasurementProcessUiHandler::connectSummaryMeasureTriplePageButtons()
{
    connect(ui->buttonNewTripleMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onNewMeasureSummaryButtonClicked);
    connect(ui->buttonReplyTripleMeasure, &QPushButton::clicked, this, &MeasurementProcessUiHandler::onReplyMeasureSummaryButtonClicked);
}

void MeasurementProcessUiHandler::connectCatalogsButtons()
{
    connect(ui->buttonTableFluids, &QPushButton::clicked, this, &MeasurementProcessUiHandler::buttonTableFluidsOnClicked);
    connect(ui->buttonSamples, &QPushButton::clicked, this, &MeasurementProcessUiHandler::buttonSamplesOnClicked);
}

void MeasurementProcessUiHandler::updateStageLabels()
{
    static const QFont normalFont = utils::stage::getNormalFont();
    static const QFont boldFont = utils::stage::getBoldFont();
    static const QPalette normalPalette;
    static const QPalette activePalette = utils::stage::getActivePalette();


    MeasurementStages::Stage currentStage = measureStateMachine->getCurrentStage();
    MeasurementStages::Stage radwagStage = radwagMeasureControler.getStage();
    bool isTripleMeasurement = ui->radioMeasureTriple->isChecked();

    const auto& currentStageToLabelMap = isTripleMeasurement ? tripleStageToLabelMap : secondStageToLabelMap;
    const auto& currentAllLabels = isTripleMeasurement ? allTripleLabels : allSecondLabels;

    for(QLabel* label : currentAllLabels)
    {
        label->setFont(normalFont);
        label->setPalette(normalPalette);
    }

    if(currentStageToLabelMap.contains(currentStage))
    {
        QLabel* activeLabel = currentStageToLabelMap[currentStage];
        activeLabel->setFont(boldFont);
        activeLabel->setPalette(activePalette);
    }

    if(static_cast<int>(radwagStage) == static_cast<int>(currentStage))
        return;

    if(currentStageToLabelMap.contains(radwagStage))
        currentStageToLabelMap[radwagStage]->setPalette(activePalette);

}

void MeasurementProcessUiHandler::initializeMappings()
{
    stageToPageMap =
    {
        {MeasurementStages::Stage::InitialData, ui->pageInitialData},
        {MeasurementStages::Stage::DryMeasure, ui->pageDryMeasure},
        {MeasurementStages::Stage::PrepareSecond, ui->pagePrepareMeasureSecond},
        {MeasurementStages::Stage::FinishSecond, ui->labelStageFluidMass},
        {MeasurementStages::Stage::PrepareTriple, ui->pagePrepareMeasureTriple},
        {MeasurementStages::Stage::SaturationMass, ui->pageSatruationMassTriple},
        {MeasurementStages::Stage::FinishTriple, ui->pageFinishSaturatedMassTriple},
        {MeasurementStages::Stage::SummaryTriple, ui->pageSummaryTriple}
    };

    tripleStageToLabelMap =
    {
        {MeasurementStages::Stage::InitialData, ui->labelStageDataTriple},
        {MeasurementStages::Stage::DryMeasure, ui->labelStageDryMassTriple},
        {MeasurementStages::Stage::PrepareTriple, ui->labelStagePrepareSaturation},
        {MeasurementStages::Stage::SaturationMass, ui->labelStageSaturationMass},
        {MeasurementStages::Stage::FinishTriple, ui->labelStageSaturatedMass},
        {MeasurementStages::Stage::SummaryTriple, ui->labelStageSummaryTriple}
    };

    secondStageToLabelMap =
    {
        {MeasurementStages::Stage::InitialData, ui->labelStageData},
        {MeasurementStages::Stage::DryMeasure, ui->labelStageDryMass},
        {MeasurementStages::Stage::PrepareSecond, ui->labelStagePreparation},
        {MeasurementStages::Stage::FinishSecond, ui->labelStageFluidMass},
        {MeasurementStages::Stage::SummarySecond, ui->labelStageSummary}
    };

    allTripleLabels =
    {
        ui->labelStageDataTriple,
        ui->labelStageDryMassTriple,
        ui->labelStagePrepareSaturation,
        ui->labelStageSaturationMass,
        ui->labelStageSaturatedMass,
        ui->labelStageSummaryTriple
    };

    allSecondLabels =
    {
        ui->labelStageData,
        ui->labelStageDryMass,
        ui->labelStagePreparation,
        ui->labelStageFluidMass,
        ui->labelStageSummary
    };
}

void MeasurementProcessUiHandler::onStartMeasureButtonClicked()
{
    radwagMeasureControler.beginNewMeasure();
    measureStateMachine->goToNextStage();
}

