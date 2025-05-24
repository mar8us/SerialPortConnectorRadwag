#include "sample_manager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

SampleManager::SampleManager(QObject *parent)
    : QObject(parent)
{
    loadSamples();
}

const QMap<QString, Sample> &SampleManager::getSamples() const
{
    return samples;
}

void SampleManager::setSamples(const QMap<QString, Sample> &newSamples)
{
    samples = newSamples;
    saveSamples();
    emit samplesChanged();
}

Sample SampleManager::getSample(const QString &number) const
{
    return samples.value(number);
}

bool SampleManager::sampleExists(const QString &number) const
{
    return samples.contains(number);
}

bool SampleManager::addSample(const Sample &sample)
{
    QString number = sample.getId();

    if(samples.contains(number))
        return false;

    samples.insert(number, sample);
    saveSamples();
    emit sampleAdded(number);
    emit samplesChanged();

    return true;
}

bool SampleManager::updateSample(const Sample &sample)
{
    QString number = sample.getId();

    if(!samples.contains(number))
        return false;

    samples[number] = sample;
    saveSamples();
    emit sampleUpdated(number);
    emit samplesChanged();

    return true;
}

bool SampleManager::removeSample(const QString &number)
{
    if(!samples.contains(number))
        return false;

    samples.remove(number);
    saveSamples();
    emit sampleRemoved(number);
    emit samplesChanged();

    return true;
}

bool SampleManager::reloadSamples()
{
    loadSamples();
    return true;
}

void SampleManager::loadSamples()
{
    QString filePath = getSamplesFilePath();
    QFile file(filePath);

    if(!file.exists())
        return;

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open samples file for reading.");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if(doc.isNull())
    {
        qWarning("Failed to parse samples JSON.");
        return;
    }

    if(!doc.isArray())
    {
        qWarning("Samples JSON is not an array.");
        return;
    }

    samples.clear();
    QJsonArray sampleArray = doc.array();

    for(const QJsonValue &value : sampleArray)
    {
        if (!value.isObject())
            continue;
        Sample sample;
        sample.fromJson(value.toObject());
        samples.insert(sample.getId(), sample);
    }

    emit samplesChanged();
}

bool SampleManager::saveSamples()
{
    QString filePath = getSamplesFilePath();

    QDir dir = QFileInfo(filePath).dir();
    if(!dir.exists())
        dir.mkpath(".");

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning("Nie można utworzyć pliku do zapisu próbek. Uruchom program ponownie z uprawnieniami administratora.");
        return false;
    }

    QJsonArray sampleArray;

    for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
        sampleArray.append(it.value().toJson());

    QJsonDocument doc(sampleArray);
    file.write(doc.toJson());
    file.close();

    return true;
}

QString SampleManager::getSamplesFilePath() const
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QString("%1/samples.json").arg(dataLocation);
}
