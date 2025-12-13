#ifndef SIEVE_ANALYSIS_MODULE_H
#define SIEVE_ANALYSIS_MODULE_H

#include "sieve_analysis_process_ui_handler.h"
#include <QObject>

class MainWindow;

namespace Ui {
class MainWindow;
}

class SieveAnalysisModule : public QObject
{
    Q_OBJECT
public:
    explicit SieveAnalysisModule(MainWindow *mainWindow);

    void initialize();

private:
    MainWindow *mainWindow;
    Ui::MainWindow *ui;

    SieveAnalysisProcessUiHandler processUiHandler;
};

#endif
