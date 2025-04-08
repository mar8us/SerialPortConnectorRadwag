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

    const QMap<QString, std::shared_ptr<const Measurement>>& getMeasurements() const;
    const QMap<QString, MeasurementResults>& getResults() const;

    std::shared_ptr<const Measurement> getMeasurement(const QString& id) const;
    bool measurementExists(const QString& id) const;
    bool addMeasurement(const std::shared_ptr<Measurement> &measurement);
    bool removeMeasurement(const QString& id);

    MeasurementResults getResults(const QString& measurementId) const;
    bool hasResults(const QString& measurementId) const;
    bool calculateResults(const QString& measurementId);

    QVector<std::shared_ptr<const Measurement>> getMeasurementsForSample(const QString& sampleId) const;

    bool reloadData();
    QString generateMeasurementId() const;

signals:
    void measurementsChanged();
    void measurementAdded(const QString& id);
    void measurementUpdated(const QString& id);
    void measurementRemoved(const QString& id);
    void resultsCalculated(const QString& measurementId);

private:
    QMap<QString, std::shared_ptr<const Measurement>> measurements;
    QMap<QString, MeasurementResults> results;

    MeasurementManager(const MeasurementManager&) = delete;
    MeasurementManager& operator=(const MeasurementManager&) = delete;

    void loadMeasurements();
    bool saveMeasurements();
    void loadResults();
    bool saveResults();
    QString getMeasurementsFilePath() const;
    QString getResultsFilePath() const;
};

#endif // MEASUREMENT_MANAGER_H
