#ifndef HYDROSTATIC_MEASURE_MODULE_H
#define HYDROSTATIC_MEASURE_MODULE_H

#include <QObject>
#include "data_holder/hydrostatic_data_holder.h"
#include "measurement_process_ui_handler.h"
#include "measurement_library_ui_handler.h"
#include "../../radwag/manual_measurements_handler.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class HydrostaticMeasurementModule : public QObject
{
    Q_OBJECT
public:
    explicit HydrostaticMeasurementModule(MainWindow *mainWindow);
    ~HydrostaticMeasurementModule();

    void initialize();

    bool hasActiveMeasurement() const;

private:
    void connectSignals();

protected:
    MainWindow *mainWindow;
    Ui::MainWindow *ui;
    HydrostaticDataHolder dataHolder;

    std::shared_ptr<MeasurementController> radwagMeasureControler;
    MeasurementProcessUiHandler processUiHandler;
    MeasurementLibraryUiHandler libraryUiHandler;
    ManualMeasurementsHandler manualUiHandler;
};

#endif
