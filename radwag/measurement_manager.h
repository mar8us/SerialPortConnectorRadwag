#ifndef MEASUREMENT_MANAGER_H
#define MEASUREMENT_MANAGER_H

#include <QObject>
#include <QMap>
#include "measurement.h"
#include "measurement_results.h"

class MeasurementManager : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementManager(QObject *parent = nullptr);
    ~MeasurementManager()
    {
        saveMeasurements();
    }

    const QMap<QString, std::shared_ptr<const Measurement>>& getMeasurements() const;
    const QMap<QString, MeasurementResults>& getResultsMap() const;

    std::shared_ptr<const Measurement> getMeasurement(const QString& id) const;
    bool measurementExists(const QString& id) const;
    bool addMeasurement(const std::shared_ptr<Measurement> &measurement);
    bool removeMeasurement(const QString& id);

    QVector<std::shared_ptr<const Measurement>> getMeasurementsForSample(const QString& sampleId) const;

    bool reloadData();

    void loadMeasurements();
    bool saveMeasurements();

signals:
    void measurementsChanged();
    void measurementAdded(const QString& id);
    void measurementUpdated(const QString& id);
    void measurementRemoved(const QString& id);
    void resultsCalculated(const QString& measurementId);

private:
    QMap<QString, std::shared_ptr<const Measurement>> measurements;

    MeasurementManager(const MeasurementManager&) = delete;
    MeasurementManager& operator=(const MeasurementManager&) = delete;

    QString getMeasurementsFilePath() const;
};

#endif // MEASUREMENT_MANAGER_H
