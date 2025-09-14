#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QString>
#include <QDateTime>
#include "../fluid_tabels/fluid.h"
#include "../sample/sample.h"
#include "measurement_results.h"
#include <QDebug>
#include "../sample/sample_manager.h"

enum class MeasurementType
{
    None = -1,
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
    explicit Measurement(std::shared_ptr<const Sample> sample);
    explicit Measurement(const Measurement& other);
    ~Measurement();

    QString getId() const;

    MeasurementType getType() const;
    MeasurementStages::Stage getCurrentStage() const;
    MeasurementStatus getStatus() const;
    SaturationMethod getSaturationMethod() const;
    int getSaturationTime() const;
    QDateTime getDate() const;
    QDateTime getEndDate() const;
    QString getAuthor() const;

    void setType(MeasurementType newType);
    void setStage(MeasurementStages::Stage);
    void setStatus(MeasurementStatus newStatus);
    void setSaturationMethod(SaturationMethod method);
    void setSaturationTime(int saturationTimeMiuntes);
    void setSaturationBeginDate(QDateTime beginDate);
    QDateTime getSaturationBeginDate() const;

    void setDate(const QDateTime& newDate);
    void setEndDate(const QDateTime& newDate);
    void setAuthor(const QString &authorName);

    std::shared_ptr<const Sample> getSample() const;
    QString getSampleName() const;
    double getSampleMaterialDensity() const;
    double getSampleDryMass() const;
    double getSampleInFluidMass() const;
    double getSampleSaturatedMass() const;
    void setSample(std::shared_ptr<const Sample> newSample);
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

    bool calculateResults();
    const MeasurementResults &getResults() const;
    bool hasResults() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json, const SampleManager *sampleManager);

private:
    QString id;
    MeasurementType type;
    MeasurementStages::Stage stage;
    MeasurementStatus status;
    SaturationMethod saturationMethod;
    int saturationTimeMin;
    QDateTime saturationBeginDate;
    QDateTime date;
    QDateTime endDate;
    QString author;

    double fluidTemperature;
    double sampleDryMass;
    double sampleInFluidMass;
    double sampleSaturatedMass;

    std::shared_ptr<const Sample> sample;
    Fluid fluid;

    MeasurementResults results;
};

#endif // MEASUREMENT_H
