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

QString SampleManager::generateSampleNumber() const
{
    QDateTime currentDate = QDateTime::currentDateTime();
    QString dateSuffix = QString("%1-%2-%3")
                             .arg(currentDate.date().day(), 2, 10, QChar('0'))
                             .arg(currentDate.date().month(), 2, 10, QChar('0'))
                             .arg(currentDate.date().year() % 100, 2, 10, QChar('0'));

    int maxNumber = 0;
    QString prefix = "PRB-";

    for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
    {
        QString sampleNumber = it.key();
        if(sampleNumber.startsWith(prefix))
        {
            int dashPos = sampleNumber.indexOf('-', prefix.length());
            if(dashPos > prefix.length())
            {
                QString numPart = sampleNumber.mid(prefix.length(), dashPos - prefix.length());
                bool ok;
                int sequentialNumber = numPart.toInt(&ok);
                if(ok && sequentialNumber > maxNumber)
                    maxNumber = sequentialNumber;

            }
        }
    }

    return QString("PRB-#%1 %2").arg(maxNumber + 1).arg(dateSuffix);
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

        QJsonObject sampleObj = value.toObject();

        QString number = sampleObj["number"].toString();
        QString name = sampleObj["name"].toString();
        QString description = sampleObj["description"].toString();
        QString author = sampleObj["author"].toString();
        QDateTime date = QDateTime::fromString(sampleObj["date"].toString(), Qt::ISODate);
        QString materialName = sampleObj["materialName"].toString();
        QString materialCategory = sampleObj["materialCategory"].toString();
        QString materialDescription = sampleObj["materialDescription"].toString();
        double materialDensity = sampleObj["materialDensity"].toDouble();

        Sample sample(number, name, materialName, materialCategory, materialDescription, materialDensity, description);
        sample.setDate(date);

        samples.insert(number, sample);
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
        qWarning("Couldn't open samples file for writing.");
        return false;
    }

    QJsonArray sampleArray;

    for(auto it = samples.constBegin(); it != samples.constEnd(); it++)
    {
        const Sample &sample = it.value();

        QJsonObject sampleObj;
        sampleObj["number"] = sample.getId();
        sampleObj["name"] = sample.getName();
        sampleObj["description"] = sample.getDescription();
        sampleObj["date"] = sample.getDate().toString(Qt::ISODate);
        sampleObj["materialName"] = sample.getMaterialName();
        sampleObj["materialCategory"] = sample.getMaterialCategory();
        sampleObj["materialDescription"] = sample.getMaterialDescription();
        sampleObj["materialDensity"] = sample.getMaterialDensity();

        sampleArray.append(sampleObj);
    }

    QJsonDocument doc(sampleArray);
    file.write(doc.toJson());
    file.close();

    return true;
}

QString SampleManager::getSamplesFilePath() const
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QString("%1/samples.json").arg(dataLocation);
}
