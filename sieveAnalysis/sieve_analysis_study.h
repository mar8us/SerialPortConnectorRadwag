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

    Sieve(double meshSize, const QString& description = "")
        : meshSize(meshSize), description(description), emptyMass(0.0), fullMass(0.0), isWeighed(false), isFinalWeighed(false)
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


    QString getStudyName() const;
    void setStudyName(const QString& name);

    QString getMaterialType() const;
    void setMaterialType(const QString& type);

    QString getAuthor() const;
    void setAuthor(const QString& name);

    QDateTime getDateTime() const;
    void setDateTime(const QDateTime& dt);

    QString getNotes() const;
    void setNotes(const QString& text);

    // === PARAMETRY PRZESIEWARKI ===
    SieverParameters getSieverParameters() const;
    void setSieverParameters(const SieverParameters& params);


    const QList<Sieve>& getSieves() const;
    void setSieves(const QList<Sieve>& sieveList);

    void addSieve(const Sieve& sieve);
    void removeSieve(int index);

    void clearSieves();
    int getSieveCount() const;

    void sortAscendingSievesBySize();

    // === MASA PRÓBKI ===
    double getSampleMass() const;

    void setSampleMass(double mass);
    bool isSampleWeighed() const;
    void setSampleWeighed(bool weighed);

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
    QString author;
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
