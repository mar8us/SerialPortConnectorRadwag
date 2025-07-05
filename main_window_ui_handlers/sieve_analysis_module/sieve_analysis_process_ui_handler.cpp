#include "sieve_analysis_process_ui_handler.h"
#include "../../sieveAnalysis/sieve_analysis_stages.h"
#include "../../main_window.h"
#include "../../utils.h"

SieveAnalysisProcessUiHandler::SieveAnalysisProcessUiHandler(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
{

}

void SieveAnalysisProcessUiHandler::initialize()
{
    connectSignals();
    initializeMappings();
    navigateToStage(SieveAnalysisStages::Stage::InitialSieveData);
}

void SieveAnalysisProcessUiHandler::goToNextSieveStage()
{
    SieveAnalysisStages::Stage currentStage = ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();

    if(!validateUIDataForStage(ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>()))
        return;

    SieveAnalysisStages::Stage nextStage = SieveAnalysisStages::nextStage(currentStage);
    navigateToStage(nextStage);
}

void SieveAnalysisProcessUiHandler::goToPreviousSieveStage()
{
    SieveAnalysisStages::Stage currentStage = ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();
    SieveAnalysisStages::Stage prevStage = SieveAnalysisStages::previousStage(currentStage);
    navigateToStage(prevStage);
}

void SieveAnalysisProcessUiHandler::navigateToStage(SieveAnalysisStages::Stage targetStage)
{
    if(!stageToPageMap.contains(targetStage))
    {
        qWarning() << "Brak strony: " << static_cast<int>(targetStage);
        return;
    }

    ui->stackedWidgetSieveAnalysis->setCurrentWidget(stageToPageMap[targetStage]);
    ui->stackedWidgetSieveAnalysis->setProperty("currentStage", QVariant::fromValue(targetStage));
    updateSieveStageLabels();
}

SieveAnalysisStages::Stage SieveAnalysisProcessUiHandler::getCurrentStage() const
{
    return ui->stackedWidgetSieveAnalysis->property("currentStage").value<SieveAnalysisStages::Stage>();
}

bool SieveAnalysisProcessUiHandler::validateUIDataForStage(SieveAnalysisStages::Stage stage) const
{
    switch(stage)
    {
        case SieveAnalysisStages::Stage::InitialSieveData:
            return true;

        case SieveAnalysisStages::Stage::ConfigurationSieve:
            return true;

        case SieveAnalysisStages::Stage::InitialWeighing:
            return true;

        case SieveAnalysisStages::Stage::Sieving:
            return true;

        case SieveAnalysisStages::Stage::FinalWeighing:
            return true;

        case SieveAnalysisStages::Stage::Summary:
            return true;

        default:
            return false;
    }
}

void SieveAnalysisProcessUiHandler::updateSieveStageLabels()
{
    static const QFont normalFont = utils::stage::getNormalFont();
    static const QFont boldFont = utils::stage::getBoldFont();
    static const QPalette normalPalette;
    static const QPalette activePalette = utils::stage::getActivePalette();

    SieveAnalysisStages::Stage currentStage = getCurrentStage();

    for (auto it = stageToLabelMap.constBegin(); it != stageToLabelMap.constEnd(); it++)
    {
        QLabel* label = it.value();
        label->setFont(normalFont);
        label->setPalette(normalPalette);
    }

    if (stageToLabelMap.contains(currentStage))
    {
        QLabel* activeLabel = stageToLabelMap[currentStage];
        activeLabel->setFont(boldFont);
        activeLabel->setPalette(activePalette);
    }
}

void SieveAnalysisProcessUiHandler::initializeMappings()
{
    stageToPageMap =
    {
        {SieveAnalysisStages::Stage::InitialSieveData, ui->pageInitialSieveData},
        {SieveAnalysisStages::Stage::ConfigurationSieve, ui->pageSieveConfiguration},
        {SieveAnalysisStages::Stage::InitialWeighing, ui->pageInitialWeighing},
        {SieveAnalysisStages::Stage::Sieving, ui->pageSieving},
        {SieveAnalysisStages::Stage::FinalWeighing, ui->pageFinalWeighing},
        {SieveAnalysisStages::Stage::Summary, ui->pageSummarySieve}
    };

    stageToLabelMap =
    {
        {SieveAnalysisStages::Stage::InitialSieveData, ui->labelSieveStageInitialData},
        {SieveAnalysisStages::Stage::ConfigurationSieve, ui->labelSieveStageConfigration},
        {SieveAnalysisStages::Stage::InitialWeighing, ui->labelSieveStageInitialWeighting},
        {SieveAnalysisStages::Stage::Sieving, ui->labelSieveStageSieving},
        {SieveAnalysisStages::Stage::FinalWeighing, ui->labelSieveStageFinalWeighting},
        {SieveAnalysisStages::Stage::Summary, ui->labelSieveStageSummary}
    };
}

void SieveAnalysisProcessUiHandler::connectSignals()
{
    connectNavSieveButtons();
}

void SieveAnalysisProcessUiHandler::connectNavSieveButtons()
{
    connect(ui->buttonStartAnalysis, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);

    connect(ui->buttonNextConfigurationSieve, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);
    connect(ui->buttonBackConfigurationSieve, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToPreviousSieveStage);

    connect(ui->buttonNextInitialSieveMeasure, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);
    connect(ui->buttonBackInitialSieveMeasure, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToPreviousSieveStage);

    connect(ui->buttonNextSieveProcess, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);
    connect(ui->buttonBackSieveProcess, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToPreviousSieveStage);

    connect(ui->buttonNextEndSieveMeasure, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);
    connect(ui->buttonBackEndSieveMeasure, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToPreviousSieveStage);

    connect(ui->buttonNewAnalysisSummarySieve, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToNextSieveStage);
    connect(ui->buttonBackSummarySieve, &QPushButton::clicked, this, &SieveAnalysisProcessUiHandler::goToPreviousSieveStage);
}
