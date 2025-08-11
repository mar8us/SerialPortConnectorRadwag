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

const QMap<QString, std::shared_ptr<const Sample>>& SampleManager::getSamples() const
{
    return samples;
}

std::shared_ptr<const Sample> SampleManager::getSample(const QString &name) const
{
    auto it = samples.find(name);
    return (it != samples.end()) ? it.value() : nullptr;
}

bool SampleManager::addSample(std::shared_ptr<const Sample> sample)
{
    if(!sample || sample->getName().isEmpty())
        return false;

    QString name = sample->getName();
    if(samples.contains(name))
        return false;

    samples[name] = sample;
    saveSamples();
    emit sampleAdded(name);
    emit samplesChanged();
    return true;
}

bool SampleManager::updateSample(const QString &oldName, std::shared_ptr<const Sample> newSample)
{
    if(!newSample || newSample->getName().isEmpty())
        return false;

    if(!samples.contains(oldName))
        return false;

    QString newName = newSample->getName();

    samples.remove(oldName);
    samples[newName] = newSample;

    saveSamples();
    emit sampleUpdated(oldName, newName);
    emit samplesChanged();
    return true;
}

bool SampleManager::removeSample(const QString &name)
{
    if(!samples.contains(name))
        return false;

    samples.remove(name);
    saveSamples();
    emit sampleRemoved(name);
    emit samplesChanged();
    return true;
}

bool SampleManager::sampleExists(const QString &name) const
{
    return samples.contains(name);
}

bool SampleManager::reloadSamples()
{
    samples.clear();
    loadSamples();
    emit samplesChanged();
    return true;
}

QStringList SampleManager::getSampleNames() const
{
    return samples.keys();
}

int SampleManager::getSampleCount() const
{
    return samples.size();
}

void SampleManager::clear()
{
    if(!samples.isEmpty())
    {
        samples.clear();
        saveSamples();
        emit samplesChanged();
    }
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
        if(!value.isObject())
            continue;
        auto sample = std::make_shared<Sample>();
        sample->fromJson(value.toObject());
        samples[sample->getName()] = sample;
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
        sampleArray.append(it.value()->toJson());
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
