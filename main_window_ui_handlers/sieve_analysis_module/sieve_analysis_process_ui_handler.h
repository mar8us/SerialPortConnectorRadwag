#ifndef SIEVE_ANALYSIS_PROCESS_UI_HANDLER_H
#define SIEVE_ANALYSIS_PROCESS_UI_HANDLER_H

#include <qlabel.h>
#include <qobject.h>
#include "../../sieveAnalysis/sieve_analysis_stages.h"

class MainWindow;

namespace Ui {
class MainWindow;
}


class SieveAnalysisProcessUiHandler : public QObject
{
    Q_OBJECT
public:
    SieveAnalysisProcessUiHandler(MainWindow *mainWindow);

    void initialize();

private slots:
    void goToPreviousSieveStage();
    void goToNextSieveStage();

private:
    void navigateToStage(SieveAnalysisStages::Stage targetStage);
    SieveAnalysisStages::Stage getCurrentStage() const;

    bool validateUIDataForStage(SieveAnalysisStages::Stage stage) const;

    void updateSieveStageLabels();

    void initializeMappings();
    void connectSignals();
    void connectNavSieveButtons();

    MainWindow *mainWindow;
    Ui::MainWindow *ui;

    QMap<SieveAnalysisStages::Stage, QWidget*> stageToPageMap;
    QMap<SieveAnalysisStages::Stage, QLabel*> stageToLabelMap;
};

#endif // SIEVE_ANALYSIS_PROCESS_UI_HANDLER_H
