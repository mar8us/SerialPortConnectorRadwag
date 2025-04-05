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
}

const QMap<QString, Measurement>& MeasurementManager::getMeasurements() const
{
    return measurements;
}

Measurement MeasurementManager::getMeasurement(const QString& id) const
{
    return measurements.value(id);
}

bool MeasurementManager::measurementExists(const QString& id) const
{
    return measurements.contains(id);

bool MeasurementManager::addMeasurement(const Measurement& measurement)
{
    if(measurement.getId().isEmpty())
        return false;

    if(measurementExists(measurement.getId()))
        return false;

    measurements.insert(measurement.getId(), measurement);
    saveMeasurements();

    emit measurementAdded(measurement.getId());
    emit measurementsChanged();

    return true;
}

bool MeasurementManager::updateMeasurement(const Measurement& measurement)
{
    if(measurement.getId().isEmpty() || !measurementExists(measurement.getId()))
        return false;

    measurements[measurement.getId()] = measurement;
    saveMeasurements();

    emit measurementUpdated(measurement.getId());
    emit measurementsChanged();

    return true;
}

bool MeasurementManager::removeMeasurement(const QString& id)
{
    if(!measurementExists(id))
        return false;

    measurements.remove(id);
    saveMeasurements();

    emit measurementRemoved(id);
    emit measurementsChanged();

    return true;
}

QList<Measurement> MeasurementManager::getMeasurementsForSample(const QString& sampleId) const
{
    QList<Measurement> sampleMeasurements;

    for(const Measurement& measurement : measurements)
        if(measurement.getSampleId() == sampleId)
            sampleMeasurements.append(measurement);

    return sampleMeasurements;
}

bool MeasurementManager::reloadData()
{
    loadMeasurements();
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
        Measurement measurement;
        measurement.fromJson(obj);
        measurements.insert(measurement.getId(), measurement);
    }
}

bool MeasurementManager::saveMeasurements()
{
    QJsonArray measurementsArray;
    for(const Measurement& measurement : measurements)
    {
        measurementsArray.append(measurement.toJson());
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
