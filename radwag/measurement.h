#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QString>
#include <QDateTime>
#include "../fluid_tabels/fluid.h"
#include "../sample/sample.h"

enum class MeasurementType
{
    TwoStage,
    ThreeStage
};

namespace MeasurementStages
{
Q_NAMESPACE

enum class Stage
{
    InitialData,
    DryMeasure,
    PrepareSecond,
    PrepareTriple,
    FinishSecond,
    SaturationMass,
    FinishTriple,
};
Q_ENUM_NS(Stage)

}

enum class MeasurementStatus
{
    InProgress,
    Completed,
    Error
};

class Measurement
{
public:
    Measurement();
    Measurement(const QString& id, const Sample &sample, const Fluid &fluid);

    QString getId() const;
    MeasurementType getType() const;
    MeasurementStages::Stage getCurrentStage() const;
    MeasurementStatus getStatus() const;
    QDateTime getDate() const;
    QString getAuthor() const;
    void setId(const QString& newId);
    void setType(MeasurementType newType);
    void setStage(MeasurementStages::Stage);
    void setStatus(MeasurementStatus newStatus);
    void setDate(const QDateTime& newDate);
    void setAuthor(QString authorName);

    QString getSampleId() const;
    double getSampleMaterialDensity() const;
    double getSampleDryMass() const;
    double getSampleInFluidMass() const;
    double getSampleSaturatedMass() const;
    void setSampleDryMass(double newSampleInDryMass);
    void setSampleInFluidMass(double newSampleInFluidMass);
    void setSampleSaturatedMass(double newSampleSaturatedMass);

    QString getFluidName() const;
    double getFluidDensity() const;
    double getFluidTemperature() const;
    void setFluid(const Fluid& fluid);
    void setFluidTemperature(double newFluidTemperature);

    bool isThreeType() const;
    bool isCompleted() const;
    bool hasAllRequiredMeasurements() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString id;
    MeasurementType type;
    MeasurementStages::Stage stage;
    MeasurementStatus status;
    QDateTime date;
    QString author;

    double fluidTemperature;
    double sampleDryMass;
    double sampleInFluidMass;
    double sampleSaturatedMass;

    Sample sample;
    Fluid fluid;
};

#endif // MEASUREMENT_H
