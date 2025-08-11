#ifndef MEASUREMENT_MANAGER_H
#define MEASUREMENT_MANAGER_H

#include <QObject>
#include <QMap>
#include "measurement.h"
#include "measurement_results.h"
#include "../sample/sample_manager.h"

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
    std::shared_ptr<const Measurement> getMeasurement(const QString& id) const;
    bool measurementExists(const QString& id) const;
    bool addMeasurement(const std::shared_ptr<Measurement> &measurement);
    bool removeMeasurement(const QString& id, bool saveToFile = true);

    QVector<std::shared_ptr<const Measurement>> getMeasurementsForSample(const QString& sampleName) const;

    void loadMeasurements(std::shared_ptr<const SampleManager> sampleManager);
    bool saveMeasurements();

signals:
    void measurementsChanged();
    void measurementAdded(const QString& id);
    void measurementUpdated(const QString& id);
    void measurementRemoved(const QString& id);
    void resultsCalculated(const QString& measurementId);

public slots:
    void onRemoveMeasurementsForSample(QString sampleName);

private:
    QMap<QString, std::shared_ptr<const Measurement>> measurements;

    MeasurementManager(const MeasurementManager&) = delete;
    MeasurementManager& operator=(const MeasurementManager&) = delete;

    QString getMeasurementsFilePath() const;
};

#endif // MEASUREMENT_MANAGER_H
