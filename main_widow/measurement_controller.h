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
    bool setInitialData(MeasurementType type, const Sample &sample, const Fluid &fluid, const QString &author);
    bool replyActiveMeasure();
    bool replyMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    bool continueMeasure(const std::shared_ptr<const Measurement> &sourceMeasure);
    void endMeasure(bool reset = false);
    QString getLastMeasureId();
    const std::shared_ptr<Measurement>& getActiveMeasure();

    MeasurementStages::Stage getStage() const;

    bool setStage(MeasurementStages::Stage stage);
    bool setMeasureStatus(MeasurementStatus status);

    double getDryMass() const;
    bool setDryMass(double value);
    double getMassInFluid() const;
    bool setMassInFluid(double value);

    double getFluidTemperature();
    void setFluidTemperature(double temperature);
    double getFluidDensity();

    bool setSaturatedMass(double value);
    double getSaturatedMass();
    void setSaturationMethod(SaturationMethod method);
    void setSaturationTime(int saturationTimeMinutes);
    int getSaturationTime();
    void setSaturationBeginDate(QDateTime beginDate);
    QDateTime getSaturationBeginDate();

    bool hasActiveMeasurement() const;
    bool canCalculateResult() const;

    MeasurementResults calculateResults();

    bool isContinued();

signals:

private:
    std::shared_ptr<Measurement> measurement;
    std::shared_ptr<MeasurementManager> measurementManager;
    QString lastMeasureId;
    bool isContinueMeasure;
};

#endif // MEASUREMENTCONTROLLER_H
