#include "sieve_analysis_study.h"
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <algorithm>

SieveAnalysisStudy::SieveAnalysisStudy()
    : dateTime(QDateTime::currentDateTime())
    , sampleMass(0.0)
    , sampleWeighed(false)
{

}

SieveAnalysisStudy::~SieveAnalysisStudy()
{

}

void SieveAnalysisStudy::sortAscendingSievesBySize()
{
    std::sort(sieves.begin(), sieves.end(), [](const Sieve& a, const Sieve& b) {
        return a.meshSize > b.meshSize;
    });
}

double SieveAnalysisStudy::getTotalRecoveredMass() const
{
    double total = 0.0;
    for(const auto& sieve : sieves)
        if(sieve.isFinalWeighed)
            total += sieve.getMaterialMass();

    return total;
}

double SieveAnalysisStudy::getMassLoss() const
{
    return sampleMass - getTotalRecoveredMass();
}

double SieveAnalysisStudy::getMassLossPercentage() const
{
    if(sampleMass <= 0.0)
        return 0.0;
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

    return 100.0 - cumulative;
}

bool SieveAnalysisStudy::isConfigurationComplete() const
{
    return !studyName.isEmpty() &&
           !materialType.isEmpty() &&
           !author.isEmpty() &&
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
    obj["author"] = author;
    obj["dateTime"] = dateTime.toString(Qt::ISODate);
    obj["notes"] = notes;

    obj["sieverParameters"] = sieverParams.toJson();

    obj["sampleMass"] = sampleMass;
    obj["sampleWeighed"] = sampleWeighed;

    QJsonArray sievesArray;
    for(const auto& sieve : sieves)
        sievesArray.append(sieve.toJson());

    obj["sieves"] = sievesArray;

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
        author = obj["author"].toString();
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
        qWarning() << "Nie można otworzyć pliku do zapisu:" << filePath;
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
        qWarning() << "Nie można otworzyć pliku do odczytu:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if(doc.isNull() || !doc.isObject())
    {
        qWarning() << "Błąd dokumentu:" << filePath;
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
    sortAscendingSievesBySize();

    if(sampleMass < 0.0)
        sampleMass = 0.0;

    if(!dateTime.isValid())
        dateTime = QDateTime::currentDateTime();
}

QString SieveAnalysisStudy::getStudyName() const
{
    return studyName;
}

void SieveAnalysisStudy::setStudyName(const QString& name)
{
    studyName = name;
}


QString SieveAnalysisStudy::getMaterialType() const
{
    return materialType;
}

void SieveAnalysisStudy::setMaterialType(const QString& type)
{
    materialType = type;
}

QString SieveAnalysisStudy::getAuthor() const
{
    return author;
}

void SieveAnalysisStudy::setAuthor(const QString& name)
{
    author = name;
}


QDateTime SieveAnalysisStudy::getDateTime() const
{
    return dateTime;
}

void SieveAnalysisStudy::setDateTime(const QDateTime& dt)
{
    dateTime = dt;
}


QString SieveAnalysisStudy::getNotes() const
{
    return notes;
}

void SieveAnalysisStudy::setNotes(const QString& text)
{
    notes = text;
}


// === PARAMETRY PRZESIEWARKI ===
SieverParameters SieveAnalysisStudy::getSieverParameters() const
{
    return sieverParams;
}

void SieveAnalysisStudy::setSieverParameters(const SieverParameters& params)
{
    sieverParams = params;
}

const QList<Sieve>& SieveAnalysisStudy::getSieves() const
{
    return sieves;
}

void SieveAnalysisStudy::setSieves(const QList<Sieve>& sieveList)
{
    sieves = sieveList;
}

void SieveAnalysisStudy::addSieve(const Sieve& sieve)
{
    sieves.append(sieve);
}

void SieveAnalysisStudy::removeSieve(int index)
{
    if (index >= 0 && index < sieves.size()) sieves.removeAt(index);
}

void SieveAnalysisStudy::clearSieves()
{
    sieves.clear();
}

int SieveAnalysisStudy::getSieveCount() const
{
    return sieves.size();
}

// === MASA PRÓBKI ===
double SieveAnalysisStudy::getSampleMass() const
{
    return sampleMass;
}

void SieveAnalysisStudy::setSampleMass(double mass)
{
    sampleMass = mass;
}

bool SieveAnalysisStudy::isSampleWeighed() const
{
    return sampleWeighed;
}

void SieveAnalysisStudy::setSampleWeighed(bool weighed)
{
    sampleWeighed = weighed;
}
