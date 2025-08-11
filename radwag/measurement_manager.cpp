#include "measurement_manager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>

MeasurementManager::MeasurementManager(QObject *parent)
    : QObject(parent)
{

}

const QMap<QString, std::shared_ptr<const Measurement>>& MeasurementManager::getMeasurements() const
{
    return measurements;
}

std::shared_ptr<const Measurement> MeasurementManager::getMeasurement(const QString& id) const
{
    return measurements.value(id);
}

bool MeasurementManager::measurementExists(const QString& id) const
{
    return measurements.contains(id);
}

bool MeasurementManager::addMeasurement(const std::shared_ptr<Measurement> &measurement)
{
    if(!measurement)
        return false;

    if(measurementExists(measurement->getId()))
        return false;

    measurements.insert(measurement->getId(), measurement);

    emit measurementAdded(measurement->getId());
    emit measurementsChanged();

    return true;
}

bool MeasurementManager::removeMeasurement(const QString& id, bool saveToFile)
{
    if(!measurementExists(id))
        return false;

    measurements.remove(id);
    if(saveToFile)
        saveMeasurements();

    emit measurementRemoved(id);
    emit measurementsChanged();

    return true;
}

QVector<std::shared_ptr<const Measurement>> MeasurementManager::getMeasurementsForSample(const QString& sampleName) const
{
    QVector<std::shared_ptr<const Measurement>> sampleMeasurements;
    for(auto& measurement : measurements)
        if(measurement->getSampleName() == sampleName)
            sampleMeasurements.append(measurement);

    return sampleMeasurements;
}

void MeasurementManager::loadMeasurements(std::shared_ptr<const SampleManager> sampleManager)
{
    measurements.clear();
    QFile file(getMeasurementsFilePath());
    if(!file.exists())
        return;

    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isNull() || !document.isArray())
        return;

    QJsonArray measurementsArray = document.array();
    for(const QJsonValue& value : measurementsArray)
    {
        if(!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        auto measurement = std::make_shared<Measurement>(nullptr);
        measurement->fromJson(obj, sampleManager.get());
        measurements.insert(measurement->getId(), measurement);
    }
}

bool MeasurementManager::saveMeasurements()
{
    QJsonArray measurementsArray;
    for(const auto& pair : measurements.toStdMap())
    {
        const std::shared_ptr<const Measurement>& measurement = pair.second;
        measurementsArray.append(measurement->toJson());
    }

    QJsonDocument document(measurementsArray);
    QByteArray jsonData = document.toJson(QJsonDocument::Indented);

    QFile file(getMeasurementsFilePath());
    if(!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonData);
    file.close();

    return true;
}

void MeasurementManager::onRemoveMeasurementsForSample(QString sampleName)
{
    const auto measurements = getMeasurementsForSample(sampleName);

    for(auto it = measurements.begin(); it != measurements.end(); it++)
        removeMeasurement(it->get()->getId(), false);
    saveMeasurements();
}

QString MeasurementManager::getMeasurementsFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(appDataPath);
    if(!dir.exists())
        dir.mkpath(".");

    return dir.filePath("measurements.json");
}
