#include "measurement.h"

Measurement::Measurement(std::shared_ptr<const Sample> sample)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , type(MeasurementType::None)
    , stage(MeasurementStages::Stage::None)
    , status(MeasurementStatus::InProgress)
    , fluidTemperature(0.0)
    , sampleDryMass(0.0)
    , sampleInFluidMass(0.0)
    , sampleSaturatedMass(0.0)
    , saturationBeginDate()
    , date(QDateTime::currentDateTime())
    , saturationMethod(SaturationMethod::None)
    , saturationTimeMin(0)
    , results(MeasurementResults())
    , sample(sample)
{

}

Measurement::Measurement(const Measurement& other)
    : id(other.id)
    , type(other.type)
    , sample(other.sample)
    , fluid(other.fluid)
    , author(other.author)
    , stage(other.stage)
    , status(other.status)
    , saturationBeginDate(other.saturationBeginDate)
    , date(other.date)
    , endDate(other.endDate)
    , fluidTemperature(other.fluidTemperature)
    , sampleDryMass(other.sampleDryMass)
    , sampleInFluidMass(other.sampleInFluidMass)
    , sampleSaturatedMass(other.sampleSaturatedMass)
    , saturationMethod(other.saturationMethod)
    , saturationTimeMin(other.saturationTimeMin)
    , results(other.results)
{

}

Measurement::~Measurement()
{
}

QString Measurement::getId() const
{
    return id;
}

MeasurementType Measurement::getType() const
{
    return type;
}

MeasurementStages::Stage Measurement::getCurrentStage() const
{
    return stage;
}

MeasurementStatus Measurement::getStatus() const
{
    return status;
}

SaturationMethod Measurement::getSaturationMethod() const
{
    return saturationMethod;
}

int Measurement::getSaturationTime() const
{
    return saturationTimeMin;
}

QDateTime Measurement::getDate() const
{
    return date;
}

QDateTime Measurement::getEndDate() const
{
    return isCompleted() ? endDate : QDateTime();
}

QString Measurement::getAuthor() const
{
    return author;
}

void Measurement::setType(MeasurementType newType)
{
    type = newType;
}

void Measurement::setStage(MeasurementStages::Stage newStage)
{
    stage = newStage;
}

void Measurement::setStatus(MeasurementStatus newStatus)
{
    status = newStatus;
}

void Measurement::setSaturationMethod(SaturationMethod method)
{
    saturationMethod = method;
}

void Measurement::setSaturationTime(int saturationTimeMiuntes)
{
    saturationTimeMin = saturationTimeMiuntes;
}

void Measurement::setSaturationBeginDate(QDateTime beginDate)
{
    saturationBeginDate = beginDate;
}

QDateTime Measurement::getSaturationBeginDate() const
{
    return saturationBeginDate;
}

void Measurement::setDate(const QDateTime& newDate)
{
    date = newDate;
}

void Measurement::setEndDate(const QDateTime &newDate)
{
    endDate = newDate;
}

void Measurement::setAuthor(const QString &authorName)
{
    author = authorName;
}

std::shared_ptr<const Sample> Measurement::getSample() const
{
    return sample;
}

QString Measurement::getSampleName() const
{
    return sample ? sample->getName() : QString();
}

double Measurement::getSampleMaterialDensity() const
{
    auto sample = getSample();
    return sample ? sample->getMaterialDensity() : -1.0;
}

double Measurement::getSampleDryMass() const
{
    return sampleDryMass;
}

double Measurement::getSampleInFluidMass() const
{
    return sampleInFluidMass;
}

double Measurement::getSampleSaturatedMass() const
{
    return sampleSaturatedMass;
}

void Measurement::setSample(std::shared_ptr<const Sample> newSample)
{
    sample = newSample;
}

void Measurement::setSampleDryMass(double newSampleInDryMass)
{
    sampleDryMass = newSampleInDryMass;
}

void Measurement::setSampleInFluidMass(double newSampleInFluidMass)
{
    sampleInFluidMass = newSampleInFluidMass;
}

void Measurement::setSampleSaturatedMass(double newSampleSaturatedMass)
{
    sampleSaturatedMass = newSampleSaturatedMass;
}

const Fluid& Measurement::getFluid() const
{
    return fluid;
}

QString Measurement::getFluidName() const
{
    return fluid.getName();
}

double Measurement::getFluidDensity() const
{
    return fluid.getDensity(getFluidTemperature());
}

double Measurement::getFluidTemperature() const
{
    return fluidTemperature;
}

void Measurement::setFluid(const Fluid& newFluid)
{
    fluid = newFluid;
}

void Measurement::setFluidTemperature(double newFluidTemperature)
{
    fluidTemperature = newFluidTemperature;
}

bool Measurement::isThreeType() const
{
    return type == MeasurementType::ThreeStage;
}

bool Measurement::isCompleted() const
{
    return status == MeasurementStatus::Completed;
}

bool Measurement::hasAllRequiredMeasurements() const
{
    if(sampleDryMass <= 0.0 || sampleInFluidMass <= 0.0)
        return false;

    if(isThreeType() && sampleSaturatedMass <= 0.0)
        return false;

    if(fluid.getName().isEmpty() || fluid.getDensity(fluidTemperature) <= 0.0)
        return false;

    return true;
}

bool Measurement::calculateResults()
{
    if(!hasAllRequiredMeasurements())
        return false;

    double materialDensity = getSampleMaterialDensity();
    if(materialDensity <= 0.0)
        return false;

    return results.calculateResults(this);
}

const MeasurementResults& Measurement::getResults() const
{
    return results;
}

bool Measurement::hasResults() const
{
    return !results.getMeasurementId().isEmpty();
}

QJsonObject Measurement::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["type"] = static_cast<int>(type);
    obj["stage"] = static_cast<int>(stage);
    obj["status"] = static_cast<int>(status);
    obj["date"] = date.toString(Qt::ISODate);
    obj["endDate"] = endDate.toString(Qt::ISODate);
    obj["author"] = author;
    obj["fluidTemperature"] = fluidTemperature;
    obj["sampleDryMass"] = sampleDryMass;
    obj["sampleInFluidMass"] = sampleInFluidMass;
    obj["sampleSaturatedMass"] = sampleSaturatedMass;
    obj["saturationBeginDate"] = saturationBeginDate.toString();
    obj["sample"] = sample ? sample->getName() : QString();
    obj["fluid"] = fluid.toJson();
    obj["saturationMethod"] = static_cast<int>(saturationMethod);
    obj["saturationTimeMin"] = saturationTimeMin;
    obj["results"] = results.toJson();

    return obj;
}

void Measurement::fromJson(const QJsonObject &json, const SampleManager *sampleManager)
{
    id = json["id"].toString();
    type = static_cast<MeasurementType>(json["type"].toInt());
    stage = static_cast<MeasurementStages::Stage>(json["stage"].toInt());
    status = static_cast<MeasurementStatus>(json["status"].toInt());
    date = QDateTime::fromString(json["date"].toString(), Qt::ISODate);
    endDate = QDateTime::fromString(json["endDate"].toString(), Qt::ISODate);
    author = json["author"].toString();
    fluidTemperature = json["fluidTemperature"].toDouble();
    sampleDryMass = json["sampleDryMass"].toDouble();
    sampleInFluidMass = json["sampleInFluidMass"].toDouble();
    sampleSaturatedMass = json["sampleSaturatedMass"].toDouble();
    saturationBeginDate = QDateTime::fromString(json["saturationBeginDate"].toString());
    saturationMethod = static_cast<SaturationMethod>(json["saturationMethod"].toInt());
    saturationTimeMin = json["saturationTimeMin"].toInt();
    sample = sampleManager->getSample(json["sample"].toString());

    if(json.contains("fluid") && json["fluid"].isObject())
        fluid.fromJson(json["fluid"].toObject());

    if(json.contains("results") && json["results"].isObject())
        results.fromJson(json["results"].toObject());
}
