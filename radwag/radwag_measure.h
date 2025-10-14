#ifndef RADWAG_MEASURE_H
#define RADWAG_MEASURE_H

 #include "../settings/device_data.h"
#include <QString>

class RadwagMeasure : public DeviceData
{
public:
    enum class Unit
    {
        Unknown,
        G,      // gram
        KG,     // kilogram
        CT,     // karat
        LB,     // funt
        OZ      // uncja
    };

    explicit RadwagMeasure(const QByteArray& rawData);
    virtual ~RadwagMeasure();

    // Implementacja metody parse z klasy bazowej
    virtual bool parse() override;

    double getValue(int precision = 4) const;
    void setValue(double value);

    QString getUnitString() const;
    void setUnitString(const QString &unitStr);

    Unit getUnit() const;
    void setUnit(Unit unit);

    bool isStable() const;
    void setStable(bool stable);

private:
    double _value;
    QString _unitStr;
    Unit _unit;
    bool _stable;

    Unit parseUnitFromString(const QString &unitStr);
    bool parse(const QString &data);
};

namespace RadwagLogger
{
// Loguje surowe dane otrzymane z wagi
void logRawData(const QByteArray& rawData, const QString& filePath);

// Loguje sparsowany pomiar
void logParsedMeasure(const RadwagMeasure& measure, const QString& filePath);

// Ustawia domyślną ścieżkę do pliku logu
void setDefaultLogPath(const QString& path);

// Pobiera domyślną ścieżkę
QString getDefaultLogPath();

// Włącza/wyłącza logowanie
void enableLogging(bool enable);

// Sprawdza czy logowanie jest włączone
bool isLoggingEnabled();
}


#endif // RADWAG_MEASURE_H

// #ifndef RADWAG_MEASURE_H
// #define RADWAG_MEASURE_H
// #include "../settings/device_data.h"
// #include <QString>
// #include <QLatin1String>

// enum class Unit
// {
//     Unknown,
//     Milligrams,
//     Grams,
//     Kilograms,
// };

// class RadwagMeasure : public DeviceData
// {
// public:
//     enum class Unit {
//         Unknown,
//         G,      // gram
//         KG,     // kilogram
//         CT,     // karat
//         LB,     // funt
//         OZ      // uncja
//     };

//     RadwagMeasure(const QByteArray& rawData);

//     bool parse() override
//     {
//         // Sprawdzenie czy rawData ma wystarczającą długość
//         if (rawData.size() < 14) {
//             return false; // Za mało danych do poprawnego parsowania
//         }

//         // Pobierz numer zlecenia (pierwsze 3 znaki)
//         _order = QString::fromLatin1(rawData.mid(0, 3));

//         // Ustal czy waga jest stabilna na podstawie znaku stabilności
//         _stable = (rawData.at(3) != '-');

//         // Pobierz wartość liczbową masy (pomijamy 5. znak - _charodd)
//         QString massStr = QString::fromLatin1(rawData.mid(6, 4));
//         _value = massStr.toDouble();

//         // Ustal jednostkę
//         QByteArray unitData = rawData.mid(13, 2);
//         _unit = parseUnitFromRawData(unitData);
//         unitStr = QString::fromLatin1(unitData);
//         return true;
//     }

//     Unit parseUnitFromRawData(const QByteArray& unitData)
//     {
//         QString unitStr = QString::fromLatin1(unitData).trimmed();

//         if (unitStr == "g") return Unit::G;
//         if (unitStr == "kg") return Unit::KG;
//         if (unitStr == "ct") return Unit::CT;
//         if (unitStr == "lb") return Unit::LB;
//         if (unitStr == "oz") return Unit::OZ;
//         return Unit::Unknown;
//     }

//     QString getOrder() const
//     {
//         return _order;
//     }

//     bool isStable() const
//     {
//         return _stable;
//     }

//     double getValue() const
//     {
//         return _value;
//     }

//     Unit getUnitType() const
//     {
//         return _unit;
//     }

//     QString getUnit()
//     {
//         return unitStr;
//     }

//     bool convertTo(Unit targetUnit)
//     {

//         return true;
//     }

// private:
//     QString _order;
//     bool _stable = false;
//     double _value = 0.0;
//     Unit _unit = Unit::Unknown;
//     QString unitStr;

//     double parseValueFromRawData()
//     {
//         return 0.0;
//     }

//     Unit parseUnitFromRawData()
//     {
//         return Unit::Unknown;
//     }
// };
// #endif // RADWAG_MEASURE_H
