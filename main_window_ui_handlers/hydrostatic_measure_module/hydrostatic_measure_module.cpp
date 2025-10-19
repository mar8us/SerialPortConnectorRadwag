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
    , manualUiHandler(mainWindow, this)
{

}

HydrostaticMeasurementModule::~HydrostaticMeasurementModule()
{

}

void HydrostaticMeasurementModule::initialize()
{
    processUiHandler.initialize();
    libraryUiHandler.initialize();
    manualUiHandler.initialize();
    connectSignals();
}

bool HydrostaticMeasurementModule::hasActiveMeasurement() const
{
    return radwagMeasureControler && radwagMeasureControler->hasActiveMeasurement();
}

void HydrostaticMeasurementModule::connectSignals()
{
    connect(appCore.getScaleConnector(), &RadwagScaleConnector::radwagDataReady, &processUiHandler, &MeasurementProcessUiHandler::onRadwagMeasueReady);
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::newMeasure, &processUiHandler, &MeasurementProcessUiHandler::onNewMeasure);
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::replySelectedMeasure, &processUiHandler, &MeasurementProcessUiHandler::onReplyMeasure);
    connect(&libraryUiHandler, &MeasurementLibraryUiHandler::continueSelectedMeasure, &processUiHandler, &MeasurementProcessUiHandler::onContinueMeasure);
    connect(&processUiHandler, &MeasurementProcessUiHandler::exportMeasuresExcel, &libraryUiHandler, &MeasurementLibraryUiHandler::onExportExcelMeasures);
}
