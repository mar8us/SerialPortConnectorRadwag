#include "main_window.h"
#include "./ui_main_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectButtons();
    setIcons();
    setProperty();
    updateStageLabels();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::navigateToToolBoxPage(QWidget* page)
{
    if(page && ui->stackedWidget->indexOf(page) != -1)
        ui->stackedWidget->setCurrentWidget(page);
}

void MainWindow::goToPreviousMeasureStage()
{
    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    int prevIndex = static_cast<int>(currentStage) - 1;

    if (prevIndex >= 0)
    {
        MeasurementStage prevStage = static_cast<MeasurementStage>(prevIndex);
        ui->measureDensityStage->setCurrentIndex(prevIndex);
        ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(prevStage));
        updateStageLabels();
    }
}

void MainWindow::goToNextMeasureStage()
{
    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    int nextIndex = static_cast<int>(currentStage) + 1;

    if (nextIndex < ui->measureDensityStage->count())
    {
        MeasurementStage nextStage = static_cast<MeasurementStage>(nextIndex);
        ui->measureDensityStage->setCurrentIndex(nextIndex);
        ui->measureDensityStage->setProperty("currentStage", QVariant::fromValue(nextStage));
        updateStageLabels();
    }
}

void MainWindow::updateActionIcons(int index)
{
    ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
    ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
}

void MainWindow::updateStageLabels()
{
    QFont normalFont = ui->dataStageLabel->font();
    normalFont.setBold(false);
    normalFont.setPixelSize(12);

    QFont boldFont = normalFont;
    boldFont.setBold(true);
    boldFont.setPixelSize(13);

    QPalette activePalette;
    activePalette.setColor(QPalette::WindowText, ACTIVE_LABEL_COLOR);

    ui->dataStageLabel->setPalette(QPalette());
    ui->measureAirStageLabel->setPalette(QPalette());
    ui->prepareMeasureHydroStageLabel->setPalette(QPalette());
    ui->measureHydroStageLabel->setPalette(QPalette());
    ui->measureAirStageLabel_2->setPalette(QPalette());

    ui->dataStageLabel->setFont(normalFont);
    ui->measureAirStageLabel->setFont(normalFont);
    ui->prepareMeasureHydroStageLabel->setFont(normalFont);
    ui->measureHydroStageLabel->setFont(normalFont);
    ui->measureAirStageLabel_2->setFont(normalFont);

    MeasurementStage currentStage = ui->measureDensityStage->property("currentStage").value<MeasurementStage>();
    switch(currentStage)
    {
    case MeasurementStage::Data:
        ui->dataStageLabel->setFont(boldFont);
        ui->dataStageLabel->setPalette(activePalette);
        break;
    case MeasurementStage::AirMeasure:
        ui->measureAirStageLabel->setFont(boldFont);
        ui->measureAirStageLabel->setPalette(activePalette);
        break;
    case MeasurementStage::PrepareHydro:
        ui->prepareMeasureHydroStageLabel->setFont(boldFont);
        ui->prepareMeasureHydroStageLabel->setPalette(activePalette);
        break;
    case MeasurementStage::HydroMeasure:
        ui->measureHydroStageLabel->setFont(boldFont);
        ui->measureHydroStageLabel->setPalette(activePalette);
        break;
    case MeasurementStage::AirEndMeasure:
        ui->measureAirStageLabel_2->setFont(boldFont);
        ui->measureAirStageLabel_2->setPalette(activePalette);
        break;
    }
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

    connect(ui->buttonGoDataStage, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonGoAirStage, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonBackAirStage, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonGoPrepareStage, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonBackPrepareStage, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonGoHydroStage, &QPushButton::clicked, this, &MainWindow::goToNextMeasureStage);
    connect(ui->buttonBackHydroStage, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
    connect(ui->buttonBackAirEndStage, &QPushButton::clicked, this, &MainWindow::goToPreviousMeasureStage);
}

void MainWindow::setProperty()
{
    ui->stackedWidget->setProperty("currentStage", QVariant::fromValue(MeasurementStage::Data));
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
}
