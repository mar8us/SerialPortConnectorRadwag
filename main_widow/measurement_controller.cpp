#include "measurement_controller.h"
#include <QMessageBox>
#include <QDebug>
#include "../utils.h"

MeasurementController::MeasurementController(const std::shared_ptr<MeasurementManager> &measurementManager, QObject* parent)
    : QObject(parent)
    , measurement(nullptr)
    , measurementManager(measurementManager)
    , hasChanges(false)
{

}

bool MeasurementController::beginNewMeasure()
{
    measurement.reset(new Measurement(nullptr));
    measurement->setStage(MeasurementStages::Stage::StartMeasure);
    hasChanges = false;
    return true;
}

bool MeasurementController::replyMeasure(const std::shared_ptr<const Measurement> &sourceMeasure)
{
    beginNewMeasure();
    if(!sourceMeasure.get())
        return false;

    bool result = true;
    result &= setType(sourceMeasure->getType());
    result &= setSample(sourceMeasure->getSample());
    result &= setFluid(sourceMeasure->getFluid());
    result &= setAuthor(sourceMeasure->getAuthor());

    if(result)
        setStage(MeasurementStages::Stage::InitialData);

    hasChanges = false;
    return true;
}

bool MeasurementController::continueMeasure(const std::shared_ptr<const Measurement> &sourceMeasure)
{
    if(!sourceMeasure.get())
        return false;
    if(sourceMeasure->isCompleted())
        return false;

    measurement.reset(new Measurement(*sourceMeasure.get()));
    hasChanges = false;
    return true;
}

void MeasurementController::endMeasure()
{
    measurement.reset();
    hasChanges = false;
}

bool MeasurementController::save()
{
    if(!hasActiveMeasurement())
        return false;

    MeasurementStages::Stage currentStage = measurement->getCurrentStage();
    if(currentStage == MeasurementStages::Stage::SummarySecond || currentStage == MeasurementStages::Stage::SummaryTriple)
        measurement->setStatus(MeasurementStatus::Completed);

    measurementManager->removeMeasurement(measurement->getId());
    if(measurementManager->addMeasurement(measurement))
        hasChanges = false;

    measurement.reset(new Measurement(*measurement.get()));

    return !hasChanges;
}

bool MeasurementController::needSave()
{
    return hasActiveMeasurement() && hasChanges;
}

const std::shared_ptr<Measurement>& MeasurementController::getActiveMeasure()
{
    return measurement;
}

bool MeasurementController::hasActiveMeasurement() const
{
    return measurement && measurement->getStatus() != MeasurementStatus::Error;
}

bool MeasurementController::hasInitialData() const
{
    return measurement->getType() != MeasurementType::None
        && measurement->getSample()
        && !measurement->getFluid().getName().isEmpty()
        && !measurement->getAuthor().isEmpty();
}

MeasurementStages::Stage MeasurementController::getStage() const
{
    if(!hasActiveMeasurement())
        return MeasurementStages::Stage::None;
    return measurement->getCurrentStage();
}

MeasurementType MeasurementController::getType()
{
    if(!hasActiveMeasurement())
        return MeasurementType::None;
    return measurement->getType();
}

double MeasurementController::getDryMass() const
{
    return measurement->getSampleDryMass();
}

double MeasurementController::getMassInFluid() const
{
    return measurement->getSampleInFluidMass();
}

double MeasurementController::getSaturatedMass()
{
    return measurement->getSampleSaturatedMass();
}

int MeasurementController::getSaturationTime()
{
    return measurement->getSaturationTime();
}

QDateTime MeasurementController::getSaturationBeginDate()
{
    return measurement->getSaturationBeginDate();
}

double MeasurementController::getFluidTemperature()
{
    return measurement->getFluidTemperature();
}

double MeasurementController::getFluidDensity()
{
    return measurement->getFluidDensity();
}

bool MeasurementController::setMeasureStatus(MeasurementStatus status)
{
    if(!hasActiveMeasurement() || static_cast<int>(status) <= static_cast<int>(measurement->getStatus()))
        return false;
    measurement->setStatus(status);
    hasChanges = true;
    return true;
}

bool MeasurementController::setStage(MeasurementStages::Stage stage)
{
    if(!hasActiveMeasurement() || static_cast<int>(stage) <= static_cast<int>(measurement->getCurrentStage()))
        return false;
    measurement->setStage(stage);
    hasChanges = true;
    return true;
}

bool MeasurementController::setType(MeasurementType type)
{
    if(!hasActiveMeasurement())
        return false;

    if(measurement->getType() == type)
        return true;

    measurement->setType(type);
    hasChanges = true;
    return true;
}

bool MeasurementController::setSample(std::shared_ptr<const Sample> newSample)
{
    if(!hasActiveMeasurement())
        return false;

    if(!newSample)
    {
        measurement->setSample(nullptr);
        return true;
    }

    QString newSampleName = newSample->getName();
    if(newSampleName.isEmpty())
        return false;

    auto sample = measurement->getSample();
    if(sample && newSampleName == sample->getName())
        return true;

    measurement->setSample(newSample);
    hasChanges = true;
    return true;
}

bool MeasurementController::setFluid(const Fluid &fluid)   //ref
{
    if(!hasActiveMeasurement())
        return false;

    QString newFluidName = fluid.getName();
    if(newFluidName.isEmpty())
        return false;

    if(newFluidName == measurement->getFluidName())
        return true;

    measurement->setFluid(fluid);
    hasChanges = true;
    return true;
}

bool MeasurementController::setFluidTemperature(double temperature)
{
    if(!hasActiveMeasurement())
        return false;

    if(temperature == 0)
        return false;

    if(utils::compareDouble(temperature, measurement->getFluidTemperature()))
        return true;

    measurement->setFluidTemperature(temperature);
    hasChanges = true;
    return true;
}

bool MeasurementController::setAuthor(const QString &text)
{
    if(!hasActiveMeasurement())
        return false;

    if(text.isEmpty())
        return false;

    if(text == measurement->getAuthor())
        return true;

    measurement->setAuthor(text);
    hasChanges = true;
    return true;
}

bool MeasurementController::setDryMass(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    if(utils::compareDouble(value, measurement->getSampleDryMass()))
        return true;

    measurement->setSampleDryMass(value);
    hasChanges = true;
    return true;
}

bool MeasurementController::setMassInFluid(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    if(utils::compareDouble(value, measurement->getSampleInFluidMass()))
        return true;

    measurement->setSampleInFluidMass(value);
    hasChanges = true;
    return true;
}

bool MeasurementController::setSaturatedMass(double value)
{
    if(value < 0.0)
        return false;

    if(!hasActiveMeasurement() || measurement->isCompleted())
        return false;

    if(utils::compareDouble(value, measurement->getSampleSaturatedMass()))
        return true;

    measurement->setSampleSaturatedMass(value);
    hasChanges = true;
    return true;
}

bool MeasurementController::setSaturationMethod(SaturationMethod method)
{
    if(!hasActiveMeasurement())
        return false;

    if(method == measurement->getSaturationMethod())
        return true;

    measurement->setSaturationMethod(method);
    hasChanges = true;
    return true;
}

bool MeasurementController::setSaturationTime(int saturationTimeMinutes)
{
    if(!hasActiveMeasurement())
        return false;

    if(saturationTimeMinutes == 0)
        return false;

    if(measurement->getSaturationTime() == saturationTimeMinutes)
        return true;

    measurement->setSaturationTime(saturationTimeMinutes);
    hasChanges = true;
    return true;
}

bool MeasurementController::setSaturationBeginDate(QDateTime beginDate)
{
    if(!hasActiveMeasurement())
        return false;

    if(!beginDate.isValid())
        return false;

    if(measurement->getSaturationBeginDate() == beginDate)
        return true;

    measurement->setSaturationBeginDate(beginDate);
    hasChanges = true;
    return true;
}

MeasurementResults MeasurementController::calculateResults()
{
    if(!hasActiveMeasurement() || !measurement->hasAllRequiredMeasurements())
        return MeasurementResults();

    return measurement->calculateResults() ? measurement->getResults() : MeasurementResults();
}

bool MeasurementController::canCalculateResult() const
{
    return measurement->hasAllRequiredMeasurements();
}
