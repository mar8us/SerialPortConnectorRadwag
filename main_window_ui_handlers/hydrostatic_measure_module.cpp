#include <QMessageBox>
#include "hydrostatic_measure_module.h"
#include "../main_window.h"

HydrostaticMeasurementModule::HydrostaticMeasurementModule(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , dataHolder(mainWindow)
    , radwagMeasureControler(std::make_shared<MeasurementController>(dataHolder.measurementManager, this))
    , processUiHandler(mainWindow, *radwagMeasureControler.get(), dataHolder)
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
}

bool HydrostaticMeasurementModule::hasActiveMeasurement() const
{
    return radwagMeasureControler && radwagMeasureControler->hasActiveMeasurement();
}
