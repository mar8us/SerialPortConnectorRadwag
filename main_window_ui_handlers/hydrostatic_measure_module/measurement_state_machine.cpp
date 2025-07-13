#include "measurement_state_machine.h"

#include <QDebug>
#include <QMessageBox>
#include "../../radwag/measurement.h"


MeasurementStateMachine::MeasurementStateMachine(QStackedWidget* stackedWidget, QObject* parent)
    : StateMachine<MeasurementStages::Stage>(stackedWidget, parent)
    , isTripleMeasurement(false)
{
    setupSecondMeasurementFlow();
    setupTripleMeasurementFlow();
}

void MeasurementStateMachine::setMeasurementType(bool isTripleMeasurement)
{
    this->isTripleMeasurement = isTripleMeasurement;
}

MeasurementStages::Stage MeasurementStateMachine::getNextStage(MeasurementStages::Stage stage) const
{
    const auto& sequence = getCurrentSequence();

    auto it = std::find(sequence.begin(), sequence.end(), stage);
    if(it != sequence.end() && (it + 1) != sequence.end())
        return *(it + 1);
    return stage;
}

MeasurementStages::Stage MeasurementStateMachine::getPreviousStage(MeasurementStages::Stage stage) const
{
    const auto& sequence = getCurrentSequence();

    auto it = std::find(sequence.begin(), sequence.end(), stage);
    if(it != sequence.end() && it != sequence.begin())
        return *(it - 1);
    return stage;
}

bool MeasurementStateMachine::isLastStage(MeasurementStages::Stage stage) const
{
    const auto& sequence = getCurrentSequence();
    if(sequence.empty())
        return true;
    return stage == sequence.back();
}

bool MeasurementStateMachine::isFirstStage(MeasurementStages::Stage stage) const
{
    const auto& sequence = getCurrentSequence();
    if(sequence.empty())
        return true;
    return stage == sequence.front();
}

void MeasurementStateMachine::setupSecondMeasurementFlow()
{

    secondMeasurementSequence =
    {
        MeasurementStages::Stage::None,
        MeasurementStages::Stage::StartMeasure,
        MeasurementStages::Stage::InitialData,
        MeasurementStages::Stage::DryMeasure,
        MeasurementStages::Stage::PrepareSecond,
        MeasurementStages::Stage::FinishSecond,
        MeasurementStages::Stage::SummarySecond
    };

    addTransition(MeasurementStages::Stage::None, MeasurementStages::Stage::StartMeasure);
    addTransition(MeasurementStages::Stage::StartMeasure, MeasurementStages::Stage::InitialData);
    addTransition(MeasurementStages::Stage::InitialData, MeasurementStages::Stage::DryMeasure);
    addTransition(MeasurementStages::Stage::DryMeasure, MeasurementStages::Stage::PrepareSecond);
    addTransition(MeasurementStages::Stage::PrepareSecond, MeasurementStages::Stage::FinishSecond);
    addTransition(MeasurementStages::Stage::FinishSecond, MeasurementStages::Stage::SummarySecond);


    addTransition(MeasurementStages::Stage::InitialData, MeasurementStages::Stage::StartMeasure);
    addTransition(MeasurementStages::Stage::DryMeasure, MeasurementStages::Stage::InitialData);
    addTransition(MeasurementStages::Stage::PrepareSecond, MeasurementStages::Stage::DryMeasure);
    addTransition(MeasurementStages::Stage::FinishSecond, MeasurementStages::Stage::PrepareSecond);
}

void MeasurementStateMachine::setupTripleMeasurementFlow()
{

    tripleMeasurementSequence =
    {
        MeasurementStages::Stage::None,
        MeasurementStages::Stage::StartMeasure,
        MeasurementStages::Stage::InitialData,
        MeasurementStages::Stage::DryMeasure,
        MeasurementStages::Stage::PrepareTriple,
        MeasurementStages::Stage::SaturationMass,
        MeasurementStages::Stage::FinishTriple,
        MeasurementStages::Stage::SummaryTriple
    };

    addTransition(MeasurementStages::Stage::None, MeasurementStages::Stage::StartMeasure);
    addTransition(MeasurementStages::Stage::StartMeasure, MeasurementStages::Stage::InitialData);
    addTransition(MeasurementStages::Stage::InitialData, MeasurementStages::Stage::DryMeasure);
    addTransition(MeasurementStages::Stage::DryMeasure, MeasurementStages::Stage::PrepareTriple);
    addTransition(MeasurementStages::Stage::PrepareTriple, MeasurementStages::Stage::SaturationMass);
    addTransition(MeasurementStages::Stage::SaturationMass, MeasurementStages::Stage::FinishTriple);
    addTransition(MeasurementStages::Stage::FinishTriple, MeasurementStages::Stage::SummaryTriple);

    addTransition(MeasurementStages::Stage::InitialData, MeasurementStages::Stage::StartMeasure);
    addTransition(MeasurementStages::Stage::DryMeasure, MeasurementStages::Stage::InitialData);
    addTransition(MeasurementStages::Stage::PrepareTriple, MeasurementStages::Stage::DryMeasure);
    addTransition(MeasurementStages::Stage::SaturationMass, MeasurementStages::Stage::PrepareTriple);
    addTransition(MeasurementStages::Stage::FinishTriple, MeasurementStages::Stage::SaturationMass);
}


const std::vector<MeasurementStages::Stage>& MeasurementStateMachine::getCurrentSequence() const
{
    return isTripleMeasurement ? tripleMeasurementSequence : secondMeasurementSequence;
}
