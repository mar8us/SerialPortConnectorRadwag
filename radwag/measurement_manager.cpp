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
    loadMeasurements();
    loadResults();
}

const QMap<QString, std::shared_ptr<const Measurement>>& MeasurementManager::getMeasurements() const
{
    return measurements;
}

const QMap<QString, MeasurementResults>& MeasurementManager::getResultsMap() const
{
    return results;
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

bool MeasurementManager::removeMeasurement(const QString& id)
{
    if(!measurementExists(id))
        return false;

    if(results.contains(id))
    {
        results.remove(id);
        saveResults();
    }

    measurements.remove(id);
    saveMeasurements();

    emit measurementRemoved(id);
    emit measurementsChanged();

    return true;
}

MeasurementResults MeasurementManager::getResults(const QString& measurementId) const
{
    return results.value(measurementId);
}

bool MeasurementManager::hasResults(const QString& measurementId) const
{
    return results.contains(measurementId);
}

bool MeasurementManager::calculateResults(const QString& measurementId)
{
    if(!measurementExists(measurementId))
        return false;

    std::shared_ptr<const Measurement> measurement = measurements[measurementId];

    if(!measurement->hasAllRequiredMeasurements())
        return false;

    MeasurementResults &measurementResults = results[measurementId];
    if(!hasResults(measurement->getId()))
        measurementResults.setMeasurementId(measurementId);

    double materialDensity = measurement->getSampleMaterialDensity();
    if(materialDensity <= 0.0)
        return false;

    if(!measurementResults.calculateResults(measurement))
        return false;

    saveResults();

    emit resultsCalculated(measurementId);

    return true;
}

QVector<std::shared_ptr<const Measurement>> MeasurementManager::getMeasurementsForSample(const QString& sampleId) const
{
    QVector<std::shared_ptr<const Measurement>> sampleMeasurements;
    for(auto& measurement : measurements)
        if(measurement->getSampleId() == sampleId)
            sampleMeasurements.append(measurement);

    return sampleMeasurements;
}

bool MeasurementManager::reloadData()
{
    loadMeasurements();
    loadResults();
    emit measurementsChanged();
    return true;
}

QString MeasurementManager::generateMeasurementId() const
{
    int currentYear = QDate::currentDate().year();
    int maxNumber = 0;

    QString yearPrefix = QString("MSR-%1-").arg(currentYear);
    for(const QString& id : measurements.keys())
    {
        if(id.startsWith(yearPrefix))
        {
            bool ok;
            int number = id.mid(yearPrefix.length()).toInt(&ok);
            if(ok && number > maxNumber)
                maxNumber = number;
        }
    }

    return QString("%1%2").arg(yearPrefix).arg(maxNumber + 1, 3, 10, QChar('0'));
}

void MeasurementManager::loadResults()
{
    results.clear();
    QFile file(getResultsFilePath());
    if(!file.exists())
        return;

    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isNull() || !document.isArray())
        return;

    QJsonArray resultsArray = document.array();
    for (const QJsonValue& value : resultsArray)
    {
        if(!value.isObject())
            continue;

        QJsonObject obj = value.toObject();
        MeasurementResults result;
        result.fromJson(obj);
        results.insert(result.getMeasurementId(), result);
    }
}

bool MeasurementManager::saveResults()
{
    QJsonArray resultsArray;
    for(const MeasurementResults& result : results)
    {
        QJsonObject obj = result.toJson();
        resultsArray.append(obj);
    }

    QJsonDocument document(resultsArray);
    QByteArray jsonData = document.toJson(QJsonDocument::Indented);

    QFile file(getResultsFilePath());
    if(!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonData);
    file.close();
    return true;
}

void MeasurementManager::loadMeasurements()
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
        auto measurement = std::make_shared<Measurement>();
        measurement->fromJson(obj);
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

QString MeasurementManager::getMeasurementsFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(appDataPath);
    if(!dir.exists())
        dir.mkpath(".");

    return dir.filePath("measurements.json");
}

QString MeasurementManager::getResultsFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(appDataPath);
    if(!dir.exists())
        dir.mkpath(".");

    return dir.filePath("measurement_results.json");
}
