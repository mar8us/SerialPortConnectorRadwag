#include "measurement_controller.h"
#include <QMessageBox>
#include <QDebug>

MeasurementController::MeasurementController(const std::shared_ptr<MeasurementManager> &measurementManager, QObject* parent)
    : QObject(parent)
    , measurement(nullptr)
    , measurementManager(measurementManager)
{

}

bool MeasurementController::beginNewMeasure()
{
    measurement.reset(new Measurement());
    measurement->setStage(MeasurementStages::Stage::StartMeasure);
    return measurementManager->addMeasurement(measurement);
}

bool MeasurementController::setInitialData(MeasurementType type, const Sample &sample, const Fluid &fluid, const QString &author)
{
    if(!hasActiveMeasurement())
        return false;
    measurement->setType(type);
    measurement->setSample(sample);
    measurement->setFluid(fluid);
    measurement->setAuthor(author);
    setStage(MeasurementStages::Stage::InitialData);
    return true;
}

bool MeasurementController::replyActiveMeasure()
{
    auto lastMeasurement = measurementManager->getMeasurement(lastMeasureId);
    if(!lastMeasurement.get())
        return false;
    beginNewMeasure();
    return setInitialData(lastMeasurement->getType(), lastMeasurement->getSample(), lastMeasurement->getFluid(), lastMeasurement->getAuthor());
}

void MeasurementController::endMeasure()
{
    lastMeasureId = measurement->getId();
    measurement.reset();
}

QString MeasurementController::getLastMeasureId()
{
    return lastMeasureId;
}

const std::shared_ptr<Measurement>& MeasurementController::getActiveMeasure()
{
    return measurement;
}

MeasurementStages::Stage MeasurementController::getStage() const
{
    if(!hasActiveMeasurement())
        return MeasurementStages::Stage::None;
    return measurement->getCurrentStage();
}

MeasurementStages::Stage MeasurementController::getPrevStage(MeasurementStages::Stage basedStage) const
{
    if(!hasActiveMeasurement())
        return MeasurementStages::Stage::None;

    MeasurementStages::Stage stage = basedStage != MeasurementStages::Stage::None ? basedStage : getStage();

    switch(stage)
    {
        case MeasurementStages::Stage::StartMeasure:
            return MeasurementStages::Stage::StartMeasure;

        case MeasurementStages::Stage::InitialData:
            return MeasurementStages::Stage::StartMeasure;

        case MeasurementStages::Stage::DryMeasure:
            return MeasurementStages::Stage::InitialData;

        case MeasurementStages::Stage::PrepareSecond:
        case MeasurementStages::Stage::PrepareTriple:
            return MeasurementStages::Stage::DryMeasure;

        case MeasurementStages::Stage::FinishSecond:
            return MeasurementStages::Stage::PrepareSecond;

        case MeasurementStages::Stage::SaturationMass:
            return MeasurementStages::Stage::PrepareTriple;

        case MeasurementStages::Stage::FinishTriple:
            return MeasurementStages::Stage::SaturationMass;

        default:
            return stage;
    }
}

MeasurementStages::Stage MeasurementController::getNextStage(MeasurementStages::Stage basedStage) const
{
    if(!hasActiveMeasurement())
        return MeasurementStages::Stage::None;

    MeasurementStages::Stage stage = basedStage != MeasurementStages::Stage::None ? basedStage : getStage();
    switch(stage)
    {
        case MeasurementStages::Stage::StartMeasure:
            return MeasurementStages::Stage::InitialData;

        case MeasurementStages::Stage::InitialData:
            return MeasurementStages::Stage::DryMeasure;

        case MeasurementStages::Stage::DryMeasure:
            return measurement->isThreeType() ? MeasurementStages::Stage::PrepareTriple : MeasurementStages::Stage::PrepareSecond;

        case MeasurementStages::Stage::PrepareSecond:
            return MeasurementStages::Stage::FinishSecond;

        case MeasurementStages::Stage::PrepareTriple:
            return MeasurementStages::Stage::SaturationMass;

        case MeasurementStages::Stage::SaturationMass:
            return MeasurementStages::Stage::FinishTriple;

        case MeasurementStages::Stage::FinishSecond:
        case MeasurementStages::Stage::FinishTriple:
            return MeasurementStages::Stage::Summary;

        default:
            return stage;
    }
}

bool MeasurementController::setStage(MeasurementStages::Stage stage)
{
    if(!hasActiveMeasurement() || static_cast<int>(stage) <= static_cast<int>(measurement->getCurrentStage()))
        return false;
    measurement->setStage(stage);
    return true;
}
bool MeasurementController::setMeasureStatus(MeasurementStatus status)
{
    if(!hasActiveMeasurement() || static_cast<int>(status) <= static_cast<int>(measurement->getStatus()))
        return false;
    measurement->setStatus(status);
    return true;
}

double MeasurementController::getDryMass() const
{
    return measurement->getSampleDryMass();
}

bool MeasurementController::setDryMass(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    measurement->setSampleDryMass(value);
    return true;
}

double MeasurementController::getMassInFluid() const
{
    return measurement->getSampleInFluidMass();
}

bool MeasurementController::setMassInFluid(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    measurement->setSampleInFluidMass(value);
    return true;
}

bool MeasurementController::setSaturatedMass(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    measurement->setSampleSaturatedMass(value);
    return true;
}

double MeasurementController::getSaturatedMass()
{
    return measurement->getSampleSaturatedMass();
}

void MeasurementController::setSaturationMethod(SaturationMethod method)
{
    measurement->setSaturationMethod(method);
}

double MeasurementController::getFluidTemperature()
{
    return measurement->getFluidTemperature();
}

void MeasurementController::setFluidTemperature(double temperature)
{
    measurement->setFluidTemperature(temperature);
}

double MeasurementController::getFluidDensity()
{
    return measurement->getFluidDensity();
}

void MeasurementController::setSaturationTime(int saturationTimeMinutes)
{
    measurement->setSaturationTime(saturationTimeMinutes);
}

int MeasurementController::getSaturationTime()
{
    return measurement->getSaturationTime();
}

bool MeasurementController::hasActiveMeasurement() const
{
    return measurement && measurement->getStatus() != MeasurementStatus::Error;
}

bool MeasurementController::canCalculateResult() const
{
    return measurement->hasAllRequiredMeasurements();
}

MeasurementResults MeasurementController::calculateResults()
{
    if(!hasActiveMeasurement() || !measurement->hasAllRequiredMeasurements())
        return MeasurementResults();

    measurementManager->calculateResults(measurement->getId());
    return measurementManager->getResults(measurement->getId());
}
