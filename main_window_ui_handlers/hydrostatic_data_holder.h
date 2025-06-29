#ifndef HYDROSTATIC_DATA_HOLDER_H
#define HYDROSTATIC_DATA_HOLDER_H

#include "../radwag/measurement_manager.h"
#include "../fluid_tabels/fluid_manager.h"
#include "../material_tabels/material_manager.h"
#include "../sample/sample_manager.h"

class MainWindow;

class HydrostaticDataHolder
{
public:
    explicit HydrostaticDataHolder(MainWindow *mainWindow);

    std::shared_ptr<MeasurementManager> measurementManager;
    std::shared_ptr<FluidManager> fluidManager;
    std::shared_ptr<MaterialManager> materialManager;
    std::shared_ptr<SampleManager> sampleManager;
};

#endif
