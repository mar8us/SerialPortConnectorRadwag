#include <QMessageBox>
#include "hydrostatic_measure_module.h"
#include "../../main_window.h"
#include "../../app_core.h"

HydrostaticMeasurementModule::HydrostaticMeasurementModule(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , dataHolder(mainWindow)
    , radwagMeasureControler(std::make_shared<MeasurementController>(dataHolder.measurementManager, this))
    , processUiHandler(mainWindow, *radwagMeasureControler.get(), dataHolder, this)
    , libraryUiHandler(mainWindow, dataHolder)
{

}

HydrostaticMeasurementModule::~HydrostaticMeasurementModule()
{

}

void HydrostaticMeasurementModule::initialize()
{
    processUiHandler.initialize();
    libraryUiHandler.initialize();
    connectActions();
    connectSignals();
}

bool HydrostaticMeasurementModule::hasActiveMeasurement() const
{
    return radwagMeasureControler && radwagMeasureControler->hasActiveMeasurement();
}

void HydrostaticMeasurementModule::onMeasureProcess()
{
    if(!appCore.hasConnectionWithScale())
    {
        mainWindow->navigateToToolBoxPage(ui->settingsPage);
        return;
    }

    ui->actionMeasureDensity->trigger();
    ui->tabWidgetMain->setCurrentIndex(0);
}

void HydrostaticMeasurementModule::connectActions()
{
    connect(ui->actionLibraryNewMeasure, &QAction::triggered, this, &HydrostaticMeasurementModule::onMeasureProcess);
    connect(ui->actionLibraryContinueMeasure, &QAction::triggered, this, &HydrostaticMeasurementModule::onMeasureProcess);
}

void HydrostaticMeasurementModule::connectSignals()
{
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::newMeasure, &processUiHandler, &MeasurementProcessUiHandler::onNewMeasure);
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::replySelectedMeasure, &processUiHandler, &MeasurementProcessUiHandler::onReplyMeasure);
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::continueSelectedMeasure, &processUiHandler, &MeasurementProcessUiHandler::onContinueMeasure);
}
