#include "hydrostatic_data_holder.h"
#include "../main_window.h"

HydrostaticDataHolder::HydrostaticDataHolder(MainWindow *mainWindow)
    : measurementManager(new MeasurementManager(mainWindow))
    , fluidManager(std::make_shared<FluidManager>(new FluidManager(mainWindow)))
    , materialManager(std::make_shared<MaterialManager>(new MaterialManager(mainWindow)))
    , sampleManager(std::make_shared<SampleManager>(new SampleManager(mainWindow)))
{

}
