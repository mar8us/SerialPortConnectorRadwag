#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QString>
#include <QDateTime>
#include "../fluid_tabels/fluid.h"
#include "../sample/sample.h"
#include <QDebug>

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
    None,
    StartMeasure,
    InitialData,
    DryMeasure,
    PrepareSecond,
    PrepareTriple,
    FinishSecond,
    SaturationMass,
    FinishTriple,
    SummarySecond,
    SummaryTriple
};

Q_ENUM_NS(Stage)

}

enum class MeasurementStatus
{
    InProgress,
    Completed,
    Error
};

enum class SaturationMethod
{
    None,
    BoilingInWater,
    VacuumMethod,
    LongTermSoaking
};

class Measurement
{
public:
    Measurement();
    Measurement(MeasurementType type, const Sample &sample, const Fluid &fluid, const QString &author);
    Measurement(const Measurement& other);
    ~Measurement();

    QString getId() const;

    MeasurementType getType() const;
    MeasurementStages::Stage getCurrentStage() const;
    MeasurementStatus getStatus() const;
    SaturationMethod getSaturationMethod() const;
    int getSaturationTime() const;
    QDateTime getDate() const;
    QString getAuthor() const;

    void setType(MeasurementType newType);
    void setStage(MeasurementStages::Stage);
    void setStatus(MeasurementStatus newStatus);
    void setSaturationMethod(SaturationMethod method);
    void setSaturationTime(int saturationTimeMiuntes);
    void setSaturationBeginDate(QDateTime beginDate);
    QDateTime getSaturationBeginDate();

    void setDate(const QDateTime& newDate);
    void setAuthor(const QString &authorName);

    const Sample &getSample() const;
    QString getSampleId() const;
    double getSampleMaterialDensity() const;
    double getSampleDryMass() const;
    double getSampleInFluidMass() const;
    double getSampleSaturatedMass() const;
    void setSample(const Sample &sample);
    void setSampleDryMass(double newSampleInDryMass);
    void setSampleInFluidMass(double newSampleInFluidMass);
    void setSampleSaturatedMass(double newSampleSaturatedMass);

    const Fluid& getFluid() const;
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
    SaturationMethod saturationMethod;
    int saturationTimeMin;
    QDateTime saturationBeginDate;
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
