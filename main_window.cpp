#include "main_window.h"
#include "./ui_main_window.h"

#include <QMessageBox>

#include "sieveAnalysis/sieve_analysis_stages.h"
#include "tooltip/tooltip_manager.h"
#include "app_core.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , deviceManagerUiHandler(this)
    , hydrostaticMeasurementModule(this)
    , sieveAnalysisModule(this)
{
    initControls();
    connectButtons();
    connectScaleSignals();
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

    deviceManagerUiHandler.initialize();
    hydrostaticMeasurementModule.initialize();
    sieveAnalysisModule.initialize();

    TooltipManager& tooltipManager = TooltipManager::getInstance();
    tooltipManager.setGlobalStyle("QToolTip { background-color: #2C3E50; color: white; }");
    tooltipManager.registerImage("info", ":/icons/image.jpg", 424, 424);
    tooltipManager.registerTooltip(ui->buttonDryMassExecuteStepOne, ui->buttonDryMassExecuteStepOne->text(), "Wyzeruj wagę wskazanym na ilustracji przyciskiem", "info", TooltipManager::IMAGE_BOTTOM);

    showMaximized();
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

bool MainWindow::showQuestion(const QString& title, const QString& message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *yesButton = msgBox.addButton("Tak", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("Nie", QMessageBox::NoRole);

    msgBox.setDefaultButton(noButton);

    msgBox.exec();

    return msgBox.clickedButton() == yesButton;
}

QString MainWindow::showSaveFileDialog(const QString &titleDialog, const QString &suggestedName, const QString& filters)
{
    QDir::homePath();
    QString filePath = QFileDialog::getSaveFileName(this, titleDialog, QDir::homePath() + "/" + suggestedName, filters);

    if(filePath.isEmpty())
        return QString();

    if(!filePath.endsWith(".xlsx", Qt::CaseInsensitive))
        filePath += ".xlsx";

    return filePath;
}

MainWindow::MessageResult MainWindow::showQuestionWithCancel(const QString& title, const QString& message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *yesButton = msgBox.addButton("Tak", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("Nie", QMessageBox::NoRole);
    QPushButton *cancelButton = msgBox.addButton("Anuluj", QMessageBox::RejectRole);

    msgBox.setDefaultButton(cancelButton);

    int result = msgBox.exec();

    if(msgBox.clickedButton() == yesButton)
        return MessageResult::Yes;
    else if (msgBox.clickedButton() == noButton)
        return MessageResult::No;
    else
        return MessageResult::Cancel;
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

void MainWindow::onConnectResult(bool connected)
{
    updateConnectonLabelsStatusBar(connected);
}

void MainWindow::onDeviceComboSelectionChanged()
{
    updateConnectonLabelsStatusBar(appCore.hasConnectionWithScale());
}

void MainWindow::updateActionIcons(int index)
{
    ui->actionSettings->setIcon(ui->stackedWidget->widget(index) == ui->settingsPage ? activeSettingsIcon : defaultSettingsIcon);
    ui->actionMeasureDensity->setIcon(ui->stackedWidget->widget(index) == ui->measureDensityPage ? activeRadwagIcon : defaultRadwagIcon);
    ui->actionSieveAnalysis->setIcon(ui->stackedWidget->widget(index) == ui->sieveAnalysisPage ? activeSieveIcon : defaultSieveIcon);
}

void MainWindow::updateConnectonLabelsStatusBar(bool connectionStatus)
{
    if(connectionStatus)
    {
        ui->labelDeviceNameStatusBar->setText("Urządzenie: " + ui->comboBoxSelectDevice->currentText());
        ui->labelConnectionStatusStatusBar->setText("Status: <font color='#2ECC71'><b>Połączono</b></font>");
        ui->comboBoxSelectDevice->setEnabled(false);
    }
    else
    {
        QString deviceName = ui->comboBoxSelectDevice->currentText();
        ui->labelDeviceNameStatusBar->setText("Urządzenie: " + (deviceName.isEmpty() ? "Nie wybrano" : ui->comboBoxSelectDevice->currentText()));
        ui->labelConnectionStatusStatusBar->setText("Status: <font color='#E74C3C'><b>Brak połączenia</b></font>");
        ui->comboBoxSelectDevice->setEnabled(true);
    }
    ui->labelDeviceNameStatusBar->setStyleSheet("background-color: transparent; border: none;");
    ui->labelConnectionStatusStatusBar->setStyleSheet("background-color: transparent; border: none;");
}

void MainWindow::initControls()
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->labelDeviceNameStatusBar);
    ui->statusbar->addPermanentWidget(ui->labelConnectionStatusStatusBar);
    setIcons();
    setPalette();
    navigateToToolBoxPage(ui->measureDensityPage);
    updateActionIcons(0);
    updateConnectonLabelsStatusBar(false);
    // ui->tabWidgetMain->setTabVisible(2, false);
    // ui->tabWidgetMain->setTabVisible(3, false);
    QTimer::singleShot(0, this, &MainWindow::resizeAllTablesColumnsToContents);
}

void MainWindow::resizeAllTablesColumnsToContents()
{
    QList<QTableWidget*> tables = this->findChildren<QTableWidget*>();

    for(QTableWidget* table : tables)
        table->resizeColumnsToContents();
}

void MainWindow::setIcons()
{
    defaultSettingsIcon = QIcon(":/icons/settings_white.png");
    activeSettingsIcon = QIcon(":/icons/settings_selected.png");
    defaultRadwagIcon = QIcon(":/icons/balance_white.png");
    activeRadwagIcon = QIcon(":/icons/balance_selected.png");
    defaultSieveIcon = QIcon(":/icons/funnel_white.png");
    activeSieveIcon = QIcon(":/icons/funnel_selected.png");
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

void MainWindow::connectButtons()
{
    connectMainNavButtons();
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
    connect(ui->comboBoxSelectDevice, &QComboBox::currentIndexChanged, this, &MainWindow::onDeviceComboSelectionChanged);
}


