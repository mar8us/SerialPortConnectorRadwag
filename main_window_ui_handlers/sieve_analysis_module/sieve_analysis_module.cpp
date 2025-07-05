#include "sieve_analysis_module.h"
#include "../../main_window.h"

SieveAnalysisModule::SieveAnalysisModule(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , processUiHandler(mainWindow)
{

}

void SieveAnalysisModule::initialize()
{
    processUiHandler.initialize();
}
