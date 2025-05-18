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

    bool beginNewMeasure(MeasurementType type);
    bool setInitialData(MeasurementType type, const Sample &sample, const Fluid &fluid, const QString &author);
    bool replyActiveMeasure();
    void endMeasure();
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

    bool hasActiveMeasurement() const;
    bool canCalculateResult() const;

    MeasurementResults calculateResults();

signals:
    void measurementChanged();
    void measurementSaved();
    void measurementLoaded();
    void stageChanged(int stageIndex);
    void measurementCompleted();

private:
    std::shared_ptr<Measurement> measurement;
    std::shared_ptr<MeasurementManager> measurementManager;
    QString lastMeasureId;
};

#endif // MEASUREMENTCONTROLLER_H
