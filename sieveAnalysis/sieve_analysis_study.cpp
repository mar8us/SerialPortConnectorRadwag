#include "sieve_analysis_study.h"
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <algorithm>

SieveAnalysisStudy::SieveAnalysisStudy()
    : temperature(20.0)
    , dateTime(QDateTime::currentDateTime())
    , sampleMass(0.0)
    , sampleWeighed(false)
{

}

SieveAnalysisStudy::~SieveAnalysisStudy()
{

}

void SieveAnalysisStudy::sortSievesBySize()
{
    std::sort(sieves.begin(), sieves.end(), [](const Sieve& a, const Sieve& b) {
        return a.meshSize > b.meshSize; // Od największych do najmniejszych
    });
}

double SieveAnalysisStudy::getTotalRecoveredMass() const
{
    double total = 0.0;
    for (const auto& sieve : sieves)
        if (sieve.isFinalWeighed)
            total += sieve.getMaterialMass();

    return total;
}

double SieveAnalysisStudy::getMassLoss() const
{
    return sampleMass - getTotalRecoveredMass();
}

double SieveAnalysisStudy::getMassLossPercentage() const
{
    if (sampleMass <= 0.0) return 0.0;
    return (getMassLoss() / sampleMass) * 100.0;
}

double SieveAnalysisStudy::getFractionPercentage(int sieveIndex) const
{
    if(sieveIndex < 0 || sieveIndex >= sieves.size() || sampleMass <= 0.0)
        return 0.0;

    if(!sieves[sieveIndex].isFinalWeighed)
        return 0.0;

    return (sieves[sieveIndex].getMaterialMass() / sampleMass) * 100.0;
}

double SieveAnalysisStudy::getCumulativePercentage(int sieveIndex) const
{
    if(sieveIndex < 0 || sieveIndex >= sieves.size())
        return 0.0;

    double cumulative = 0.0;
    for(int i = 0; i <= sieveIndex; ++i)
        cumulative += getFractionPercentage(i);

    return 100.0 - cumulative; // Procent przechodzącej przez sito
}

bool SieveAnalysisStudy::isConfigurationComplete() const
{
    return !studyName.isEmpty() &&
           !materialType.isEmpty() &&
           !performer.isEmpty() &&
           !sieves.isEmpty();
}

bool SieveAnalysisStudy::areEmptySievesWeighed() const
{
    for(const auto& sieve : sieves)
        if(!sieve.isWeighed)
            return false;

    return !sieves.isEmpty() && sampleWeighed;
}

bool SieveAnalysisStudy::areFinalSievesWeighed() const
{
    for(const auto& sieve : sieves)
        if(!sieve.isFinalWeighed)
            return false;

    return !sieves.isEmpty();
}

bool SieveAnalysisStudy::isStudyComplete() const
{
    return isConfigurationComplete() &&
           areEmptySievesWeighed() &&
           areFinalSievesWeighed();
}

QJsonObject SieveAnalysisStudy::toJson() const
{
    QJsonObject obj;

    obj["studyName"] = studyName;
    obj["materialType"] = materialType;
    obj["temperature"] = temperature;
    obj["performer"] = performer;
    obj["dateTime"] = dateTime.toString(Qt::ISODate);
    obj["notes"] = notes;

    // Parametry przesiewarki
    obj["sieverParameters"] = sieverParams.toJson();

    // Masa próbki
    obj["sampleMass"] = sampleMass;
    obj["sampleWeighed"] = sampleWeighed;

    // Sita
    QJsonArray sievesArray;
    for (const auto& sieve : sieves) {
        sievesArray.append(sieve.toJson());
    }
    obj["sieves"] = sievesArray;

    // Metadane
    obj["version"] = "1.0";
    obj["createdAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return obj;
}

bool SieveAnalysisStudy::fromJson(const QJsonObject& obj)
{
    try
    {
        studyName = obj["studyName"].toString();
        materialType = obj["materialType"].toString();
        temperature = obj["temperature"].toDouble();
        performer = obj["performer"].toString();
        dateTime = QDateTime::fromString(obj["dateTime"].toString(), Qt::ISODate);
        notes = obj["notes"].toString();

        // Parametry przesiewarki
        if(obj.contains("sieverParameters"))
            sieverParams.fromJson(obj["sieverParameters"].toObject());


        // Masa próbki
        sampleMass = obj["sampleMass"].toDouble();
        sampleWeighed = obj["sampleWeighed"].toBool();

        // Sita
        sieves.clear();
        if(obj.contains("sieves"))
        {
            QJsonArray sievesArray = obj["sieves"].toArray();
            for(const auto& value : sievesArray)
            {
                Sieve sieve;
                sieve.fromJson(value.toObject());
                sieves.append(sieve);
            }
        }

        validateData();
        return true;

    }
    catch(const std::exception& e)
    {
        qWarning() << "Error parsing JSON:" << e.what();
        return false;
    }
}

bool SieveAnalysisStudy::saveToFile(const QString& filePath) const
{
    QJsonObject obj = toJson();
    QJsonDocument doc(obj);

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    return true;
}

bool SieveAnalysisStudy::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if(doc.isNull() || !doc.isObject())
    {
        qWarning() << "Invalid JSON document in file:" << filePath;
        return false;
    }

    return fromJson(doc.object());
}

QString SieveAnalysisStudy::toJsonString() const
{
    QJsonDocument doc(toJson());
    return doc.toJson(QJsonDocument::Compact);
}

bool SieveAnalysisStudy::fromJsonString(const QString& jsonString)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

    if(doc.isNull() || !doc.isObject())
        return false;

    return fromJson(doc.object());
}

void SieveAnalysisStudy::validateData()
{
    // Sortuj sita po rozmiarze
    sortSievesBySize();

    // Waliduj temperatury
    if(temperature < 0.0 || temperature > 50.0)
        temperature = 20.0;

    // Waliduj masy
    if(sampleMass < 0.0)
        sampleMass = 0.0;

    // Waliduj daty
    if(!dateTime.isValid())
        dateTime = QDateTime::currentDateTime();

}
