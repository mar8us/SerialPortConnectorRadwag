#include "main_window.h"
#include "./ui_main_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , deviceModel(this)
    , deviceControler(deviceModel, this)
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
    deviceControler.beginNew();
    updateDevicesComboConnection();
}

void MainWindow::onEditDeviceButtonClicked()
{
    deviceControler.beginEdit(getSelectedDevice());
    updateDevicesComboConnection();
}

void MainWindow::onRemoveDeviceButtonClicked()
{
    deviceControler.beginRemove(getSelectedDevice());
    updateDevicesComboConnection();
}

void MainWindow::onDeviceComboSelectionChanged()
{
    deviceControler.setActiveDevice(ui->devicesComboConnection->currentData().value<std::shared_ptr<const Device>>());
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
    ui->devicesListView->setModel(&deviceModel);
    updateDevicesComboConnection();
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

    connect(ui->addDeviceButton, &QPushButton::clicked, this, &MainWindow::onAddDeviceButtonClicked);
    connect(ui->editDeviceButton, &QPushButton::clicked, this, &MainWindow::onEditDeviceButtonClicked);
    connect(ui->deleteDeviceButton, &QPushButton::clicked, this, &MainWindow::onRemoveDeviceButtonClicked);

    connect(ui->devicesComboConnection, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
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
