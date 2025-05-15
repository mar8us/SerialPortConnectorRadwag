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
    measurement->setId(measurementManager->generateMeasurementId());
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
    return true;
}

bool MeasurementController::replyActiveMeasure()
{
    if(!hasActiveMeasurement())
        return false;

    auto activeMeasurement = getActiveMeasure();
    beginNewMeasure();
    return setInitialData(activeMeasurement->getType(), activeMeasurement->getSample(), activeMeasurement->getFluid(), activeMeasurement->getAuthor());
}

void MeasurementController::endMeasure()
{
    measurement.reset();
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

bool MeasurementController::setStage(MeasurementStages::Stage stage)
{
    if(!hasActiveMeasurement() || static_cast<int>(stage) <= static_cast<int>(measurement->getCurrentStage()))
        return false;
    measurement->setStage(stage);
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
