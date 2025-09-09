#include "hydrostatic_data_holder.h"
#include "../../../main_window.h"
#include <QObject>

HydrostaticDataHolder::HydrostaticDataHolder(MainWindow *mainWindow)
    : fluidManager(std::make_shared<FluidManager>(mainWindow))
    , materialManager(std::make_shared<MaterialManager>(mainWindow))
    , sampleManager(std::make_shared<SampleManager>(mainWindow))
    , measurementManager(std::make_shared<MeasurementManager>(mainWindow))
{
    if(sampleManager->getSampleCount())
        measurementManager->loadMeasurements(sampleManager);
    QObject::connect(sampleManager.get(), &SampleManager::sampleRemoved, measurementManager.get(), &MeasurementManager::onRemoveMeasurementsForSample);
}
