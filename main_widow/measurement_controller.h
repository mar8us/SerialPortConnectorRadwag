#ifndef MEASUREMENTCONTROLLER_H
#define MEASUREMENTCONTROLLER_H

#include <QObject>
#include "../radwag/measurement.h"
#include "../radwag/measurement_manager.h"
#include <memory>

class MeasurementController : public QObject
{
    Q_OBJECT

public:
    MeasurementController(const std::shared_ptr<MeasurementManager> &measurementManager, QObject* parent = nullptr);

    bool beginNewMeasure();
    bool replyMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    bool continueMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    void endMeasure();
    bool save();
    bool needSave();

    const std::shared_ptr<Measurement>& getActiveMeasure();
    bool hasActiveMeasurement() const;
    bool hasInitialData() const;

    MeasurementStages::Stage getStage() const;
    MeasurementType getType();

    double getDryMass() const;
    double getMassInFluid() const;
    double getSaturatedMass();

    int getSaturationTime();
    QDateTime getSaturationBeginDate();

    double getFluidTemperature();
    double getFluidDensity();

    bool setMeasureStatus(MeasurementStatus status);
    bool setStage(MeasurementStages::Stage stage);
    bool setType(MeasurementType type);
    bool setSample(std::shared_ptr<const Sample> newSample);
    bool setFluid(const Fluid &fluid);
    bool setFluidTemperature(double temperature);
    bool setAuthor(const QString &text);

    bool setDryMass(double value);
    bool setMassInFluid(double value);
    bool setSaturatedMass(double value);

    bool setSaturationMethod(SaturationMethod method);
    bool setSaturationTime(int saturationTimeMinutes);
    bool setSaturationBeginDate(QDateTime beginDate);

    MeasurementResults calculateResults();
    bool canCalculateResult() const;

signals:

private:
    std::shared_ptr<Measurement> measurement;
    std::shared_ptr<MeasurementManager> measurementManager;
    bool hasChanges;
};

#endif // MEASUREMENTCONTROLLER_H
