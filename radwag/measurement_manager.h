#ifndef MEASUREMENT_MANAGER_H
#define MEASUREMENT_MANAGER_H

#include <QObject>
#include <QMap>
#include "measurement.h"

class MeasurementManager : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementManager(QObject *parent = nullptr);

    const QMap<QString, Measurement>& getMeasurements() const;

    Measurement getMeasurement(const QString& id) const;
    bool measurementExists(const QString& id) const;
    bool addMeasurement(const Measurement& measurement);
    bool updateMeasurement(const Measurement& measurement);
    bool removeMeasurement(const QString& id);

    QList<Measurement> getMeasurementsForSample(const QString& sampleId) const;

    bool reloadData();
    QString generateMeasurementId() const;

signals:
    void measurementsChanged();
    void measurementAdded(const QString& id);
    void measurementUpdated(const QString& id);
    void measurementRemoved(const QString& id);

private:
    QMap<QString, Measurement> measurements;

    MeasurementManager(const MeasurementManager&) = delete;
    MeasurementManager& operator=(const MeasurementManager&) = delete;

    void loadMeasurements();
    bool saveMeasurements();
    QString getMeasurementsFilePath() const;
};

#endif // MEASUREMENT_MANAGER_H
