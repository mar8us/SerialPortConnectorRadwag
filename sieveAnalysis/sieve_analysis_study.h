#ifndef SIEVE_ANALYSIS_STUDY_H
#define SIEVE_ANALYSIS_STUDY_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


struct Sieve
{
    double meshSize;        // Rozmiar siatki [mm]
    double emptyMass;       // Masa pustego sita [g] - dodawana podczas ważenia
    double fullMass;        // Masa sita z materiałem [g] - dodawana podczas ważenia końcowego
    bool isWeighed;         // Czy sito zostało zważone (puste)
    bool isFinalWeighed;    // Czy sito zostało zważone z materiałem
    QString description;    // Opis/uwagi

    Sieve()
        : meshSize(0.0), emptyMass(0.0), fullMass(0.0), isWeighed(false), isFinalWeighed(false)
    {

    }

    Sieve(double size, const QString& desc = "")
        : meshSize(size), description(desc), emptyMass(0.0), fullMass(0.0), isWeighed(false), isFinalWeighed(false)
    {

    }


    double getMaterialMass() const
    {
        return fullMass - emptyMass;
    }


    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["meshSize"] = meshSize;
        obj["description"] = description;
        obj["emptyMass"] = emptyMass;
        obj["fullMass"] = fullMass;
        obj["isWeighed"] = isWeighed;
        obj["isFinalWeighed"] = isFinalWeighed;
        return obj;
    }

    void fromJson(const QJsonObject& obj)
    {
        meshSize = obj["meshSize"].toDouble();
        description = obj["description"].toString();
        emptyMass = obj["emptyMass"].toDouble();
        fullMass = obj["fullMass"].toDouble();
        isWeighed = obj["isWeighed"].toBool();
        isFinalWeighed = obj["isFinalWeighed"].toBool();
    }
};


struct SieverParameters
{
    int amplitude;      // Amplituda (1-100)
    int vibrations;     // Wibracje (1-100)
    int breakTime;      // Przerwa [s] (0-300)
    int workTime;       // Czas pracy [min] (10-1440)

    SieverParameters()
        : amplitude(50), vibrations(20), breakTime(0), workTime(30)
    {

    }

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["amplitude"] = amplitude;
        obj["vibrations"] = vibrations;
        obj["breakTime"] = breakTime;
        obj["workTime"] = workTime;
        return obj;
    }

    void fromJson(const QJsonObject& obj)
    {
        amplitude = obj["amplitude"].toInt();
        vibrations = obj["vibrations"].toInt();
        breakTime = obj["breakTime"].toInt();
        workTime = obj["workTime"].toInt();
    }
};


class SieveAnalysisStudy
{
public:
    SieveAnalysisStudy();
    ~SieveAnalysisStudy();


    QString getStudyName() const
    {
        return studyName;
    }

    void setStudyName(const QString& name)
    {
        studyName = name;
    }


    QString getMaterialType() const
    {
        return materialType;
    }

    void setMaterialType(const QString& type)
    {
        materialType = type;
    }


    double getTemperature() const
    {
        return temperature;
    }

    void setTemperature(double temp)
    {
        temperature = temp;
    }


    QString getPerformer() const
    {
        return performer;
    }

    void setPerformer(const QString& name)
    {
        performer = name;
    }


    QDateTime getDateTime() const
    {
        return dateTime;
    }

    void setDateTime(const QDateTime& dt)
    {
        dateTime = dt;
    }


    QString getNotes() const
    {
        return notes;
    }

    void setNotes(const QString& text)
    {
        notes = text;
    }


    // === PARAMETRY PRZESIEWARKI ===
    SieverParameters getSieverParameters() const
    {
        return sieverParams;
    }

    void setSieverParameters(const SieverParameters& params)
    {
        sieverParams = params;
    }


    QList<Sieve> getSieves() const
    {
        return sieves;
    }

    void setSieves(const QList<Sieve>& sieveList)
    {
        sieves = sieveList;
    }

    void addSieve(const Sieve& sieve)
    {
        sieves.append(sieve);
    }


    void addSieve(double meshSize, const QString& description = "")
    {
        sieves.append(Sieve(meshSize, description));
    }

    void removeSieve(int index)
    {
        if (index >= 0 && index < sieves.size()) sieves.removeAt(index);
    }

    void clearSieves()
    {
        sieves.clear();
    }

    int getSieveCount() const
    {
        return sieves.size();
    }

    // Sortuje sita od największych do najmniejszych oczek
    void sortSievesBySize();

    // === MASA PRÓBKI ===
    double getSampleMass() const
    {
        return sampleMass;
    }

    void setSampleMass(double mass)
    {
        sampleMass = mass;
    }

    bool isSampleWeighed() const
    {
        return sampleWeighed;
    }

    void setSampleWeighed(bool weighed)
    {
        sampleWeighed = weighed;
    }

    // === METODY OBLICZENIOWE ===
    // Oblicza całkowitą masę materiału odzyskanego
    double getTotalRecoveredMass() const;

    // Oblicza stratę masy w gramach
    double getMassLoss() const;

    // Oblicza stratę masy w procentach
    double getMassLossPercentage() const;

    // Oblicza udział frakcji dla konkretnego sita w %
    double getFractionPercentage(int sieveIndex) const;

    // Oblicza skumulowany udział dla konkretnego sita w %
    double getCumulativePercentage(int sieveIndex) const;

    // === STATUS BADANIA ===
    bool isConfigurationComplete() const;
    bool areEmptySievesWeighed() const;
    bool areFinalSievesWeighed() const;
    bool isStudyComplete() const;

    // === SERIALIZACJA ===
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& obj);

    // Zapisz/wczytaj z pliku
    bool saveToFile(const QString& filePath) const;
    bool loadFromFile(const QString& filePath);

    // Konwersja do/z QString (dla debugowania)
    QString toJsonString() const;
    bool fromJsonString(const QString& jsonString);

private:
    QString studyName;
    QString materialType;
    double temperature;
    QString performer;
    QDateTime dateTime;
    QString notes;

    // Parametry przesiewarki
    SieverParameters sieverParams;

    // Sita
    QList<Sieve> sieves;

    // Masa próbki
    double sampleMass;
    bool sampleWeighed;

    // Metody pomocnicze
    void validateData();
};

#endif // SIEVE_ANALYSIS_STUDY_H
