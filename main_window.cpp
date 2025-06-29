#include "main_window.h"
#include "./ui_main_window.h"

#include <QMessageBox>

#include "tooltip/tooltip_manager.h"
#include "radwag/measurement.h"
#include "sieveAnalysis/sieve_analysis_stages.h"
#include "app_core.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , deviceManagerUiHandler(this)
    , hydrostaticMeasurementModule(this)
{
    initControls();
    connectButtons();
    connectScaleSignals();
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

    deviceManagerUiHandler.initialize();
    hydrostaticMeasurementModule.initialize();

    // tooltip i.e
    TooltipManager& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setGlobalStyle("QToolTip { background-color: #2C3E50; color: white; }");
    tooltipManager.registerImage("info", ":/icons/image.jpg", 424, 424);
    tooltipManager.registerTooltip(ui->buttonDryMassExecuteStepOne, ui->buttonDryMassExecuteStepOne->text(), "Wyzeruj wagę wskazanym na ilustracji przyciskiem", "info", TooltipManager::IMAGE_BOTTOM);

    QTimer::singleShot(0, this, &MainWindow::resizeAllTablesColumnsToContents);
}

MainWindow::~MainWindow()
{
    delete ui;
}

Ui::MainWindow* MainWindow::getUi() const
{
    return ui;
}

void MainWindow::showWarning(const QString& title, const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void MainWindow::showInfo(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void MainWindow::navigateToToolBoxPage(QWidget* page)
{
    if(page && ui->stackedWidget->indexOf(page) != -1)
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::onMainPageChanged(int index)
{
    updateActionIcons(index);
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

void MainWindow::onConnectResult(bool connected)
{
    updateConnectonLabelsStatusBar(connected);
}

void MainWindow::onDeviceComboSelectionChanged()
{
    updateConnectonLabelsStatusBar(appCore.hasConnectionWithScale());
}

void MainWindow::setProperty()
{
    ui->stackedWidgetSieveAnalysis->setProperty("currentStage", QVariant::fromValue(SieveAnalysisStages::Stage::None));
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}

void MainWindow::setPalette()
{
    ui->scrollAreaInitialData->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishSecond->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinishTriple->setBackgroundRole(QPalette::Base);
    ui->scrollAreaLibrary->setBackgroundRole(QPalette::Base);
    ui->scrollAreaLibrary->setBackgroundRole(QPalette::Base);
    ui->scrollAreaSieveMain->setBackgroundRole(QPalette::Base);
    ui->scrollAreaSieveProcess->setBackgroundRole(QPalette::Base);
    ui->scrollAreaFinalWeighting->setBackgroundRole(QPalette::Base);
    ui->scrollAreaSummarySieve->setBackgroundRole(QPalette::Base);
}

void MainWindow::updateActionIcons(int index)
{
    ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
    ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
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

//SieveAnalysis

void MainWindow::goToPreviousSieveStage()
{
    SieveAnalysisStages::Stage currentStage = ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();
    SieveAnalysisStages::Stage prevStage = SieveAnalysisStages::previousStage(currentStage);
    switch(currentStage)
    {

        case SieveAnalysisStages::Stage::ConfigurationSieve:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageInitialSieveData);
            break;
        }

        case SieveAnalysisStages::Stage::InitialWeighing:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageSieveConfiguration);
            break;
        }

        case SieveAnalysisStages::Stage::Sieving:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageInitialWeighing);
            break;
        }

        case SieveAnalysisStages::Stage::FinalWeighing:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageSieving);
            break;
        }

        case SieveAnalysisStages::Stage::Summary:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageFinalWeighing);
            break;
        }

        default:
            return;
    }
    ui->stackedWidgetSieveAnalysis->setProperty("currentStage", QVariant::fromValue(prevStage));
    updateSieveStageLabels();
}

void MainWindow::goToNextSieveStage()
{
    SieveAnalysisStages::Stage currentStage = ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();
    SieveAnalysisStages::Stage nextStage = SieveAnalysisStages::nextStage(currentStage);

    switch(currentStage)
    {
        case SieveAnalysisStages::Stage::InitialSieveData:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageSieveConfiguration);
            break;
        }

        case SieveAnalysisStages::Stage::ConfigurationSieve:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageInitialWeighing);
            break;
        }

        case SieveAnalysisStages::Stage::InitialWeighing:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageSieving);
            break;
        }

        case SieveAnalysisStages::Stage::Sieving:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageFinalWeighing);
            break;
        }

        case SieveAnalysisStages::Stage::FinalWeighing:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageSummarySieve);
            break;
        }

        case SieveAnalysisStages::Stage::Summary:
        {
            ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageInitialSieveData);
            break;
        }

        default:
            return;
    }

    ui->stackedWidgetSieveAnalysis->setProperty("currentStage", QVariant::fromValue(nextStage));
    updateSieveStageLabels();
}

void MainWindow::updateSieveStageLabels()
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

    SieveAnalysisStages::Stage currentStage = ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();

    QList<QLabel*> sieveLabels =
    {
        ui->labelSieveStageInitialData,
        ui->labelSieveStageConfigration,
        ui->labelSieveStageInitialWeighting,
        ui->labelSieveStageSieving,
        ui->labelSieveStageFinalWeighting,
        ui->labelSieveStageSummary
    };

    for(QLabel* label : sieveLabels)
    {
        label->setFont(normalFont);
        label->setPalette(normalPalette);
    }

    switch (currentStage)
    {

        case SieveAnalysisStages::Stage::InitialSieveData:
            ui->labelSieveStageInitialData->setFont(boldFont);
            ui->labelSieveStageInitialData->setPalette(activePalette);
            break;

        case SieveAnalysisStages::Stage::ConfigurationSieve:
            ui->labelSieveStageConfigration->setFont(boldFont);
            ui->labelSieveStageConfigration->setPalette(activePalette);
            break;

        case SieveAnalysisStages::Stage::InitialWeighing:
            ui->labelSieveStageInitialWeighting->setFont(boldFont);
            ui->labelSieveStageInitialWeighting->setPalette(activePalette);
            break;

        case SieveAnalysisStages::Stage::Sieving:
            ui->labelSieveStageSieving->setFont(boldFont);
            ui->labelSieveStageSieving->setPalette(activePalette);
            break;

        case SieveAnalysisStages::Stage::FinalWeighing:
            ui->labelSieveStageFinalWeighting->setFont(boldFont);
            ui->labelSieveStageFinalWeighting->setPalette(activePalette);
            break;

        case SieveAnalysisStages::Stage::Summary:
            ui->labelSieveStageSummary->setFont(boldFont);
            ui->labelSieveStageSummary->setPalette(activePalette);
            break;

        default:
            break;
    }
}

void MainWindow::connectNavSieveButtons()
{
    connect(ui->buttonStartAnalysis, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);

    connect(ui->buttonNextConfigurationSieve, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);
    connect(ui->buttonBackConfigurationSieve, &QPushButton::clicked, this, &MainWindow::goToPreviousSieveStage);

    connect(ui->buttonNextInitialSieveMeasure, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);
    connect(ui->buttonBackInitialSieveMeasure, &QPushButton::clicked, this, &MainWindow::goToPreviousSieveStage);

    connect(ui->buttonNextSieveProcess, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);
    connect(ui->buttonBackSieveProcess, &QPushButton::clicked, this, &MainWindow::goToPreviousSieveStage);

    connect(ui->buttonNextEndSieveMeasure, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);
    connect(ui->buttonBackEndSieveMeasure, &QPushButton::clicked, this, &MainWindow::goToPreviousSieveStage);

    connect(ui->buttonNewAnalysisSummarySieve, &QPushButton::clicked, this, &MainWindow::goToNextSieveStage);
    connect(ui->buttonBackSummarySieve, &QPushButton::clicked, this, &MainWindow::goToPreviousSieveStage);
}

void MainWindow::resizeAllTablesColumnsToContents()
{
    QList<QTableWidget*> tables = this->findChildren<QTableWidget*>();

    for(QTableWidget* table : tables)
        table->resizeColumnsToContents();
}

void MainWindow::initControls()
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->labelDeviceNameStatusBar);
    ui->statusbar->addPermanentWidget(ui->labelConnectionStatusStatusBar);
    setIcons();
    setProperty();
    navigateToToolBoxPage(ui->measureDensityPage);
    updateActionIcons(0);
    updateConnectonLabelsStatusBar(false);
}

void MainWindow::connectButtons()
{
    connectMainNavButtons();
    connectNavSieveButtons();
}

void MainWindow::connectMainNavButtons()
{
    connect(ui->actionSettings, &QAction::triggered, this, [this]() {
        if(hydrostaticMeasurementModule.hasActiveMeasurement())
        {
            QMessageBox::warning(this, tr("Aktywny proces pomiarowy"), "Zakończ aktywny proces pomiarowy aby przejść do menadzera urządzeń.");
            return;
        }
        navigateToToolBoxPage(ui->settingsPage);
    });

    connect(ui->actionMeasureDensity, &QAction::triggered, this, [this]() {

        if(!appCore.hasConnectionWithScale() || !hydrostaticMeasurementModule.hasActiveMeasurement())
        {
            ui->stackedWidgetMainHydroMeasure->setCurrentWidget(ui->pageStartMeasure);
            ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(MeasurementStages::Stage::StartMeasure));
        }
        else if(hydrostaticMeasurementModule.hasActiveMeasurement())
            showWarning("Aktywny pomiar", "Masz aktywny pomiar hydrostatyczny. Zakończ aktualny pomiar aby wykonać kolejny.");

        navigateToToolBoxPage(ui->measureDensityPage);
    });

    connect(ui->actionSieveAnalysis, &QAction::triggered, this, [this]() {
        if(hydrostaticMeasurementModule.hasActiveMeasurement())
        {
            QMessageBox::warning(this, tr("Aktywny proces pomiarowy"), "Zakończ aktywny proces pomiarowy aby przejść do procesu analizy sitowej.");
            return;
        }
        navigateToToolBoxPage(ui->sieveAnalysisPage);
        ui->stackedWidgetSieveAnalysis->setCurrentWidget(ui->pageInitialSieveData);
        ui->stackedWidgetSieveAnalysis->setProperty("currentStage", QVariant::fromValue(SieveAnalysisStages::Stage::InitialSieveData));
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onMainPageChanged);
}

void MainWindow::connectScaleSignals()
{
    connect(appCore.getScaleConnector(), &DeviceConnector::connectionResult, this, &MainWindow::onConnectResult);
    connect(appCore.getScaleConnector(), &RadwagScaleConnector::radwagDataReady, this, &MainWindow::onRadwagMeasueReady);
    connect(ui->comboBoxSelectDevice, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
}
