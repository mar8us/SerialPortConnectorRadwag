#include "radwag_measure.h"
#include <QRegularExpression>


RadwagMeasure::RadwagMeasure(const QByteArray& rawData)
    : DeviceData(rawData), _value(0.0), _unitStr(""), _unit(Unit::Unknown), _stable(false)
{
    parse();
}

RadwagMeasure::~RadwagMeasure()
{

}

bool RadwagMeasure::parse()
{
    if(rawData.isEmpty())
        return false;

    QString data = QString::fromLatin1(rawData).trimmed();

    if(parseFlexibly(data))
        return true;

    return parseStrictFormat(rawData);
}

bool RadwagMeasure::parseFlexibly(const QString &data)
{
    QRegularExpression rxFloat("([+-]?\\d+\\.\\d+)");
    QRegularExpressionMatch match = rxFloat.match(data);

    if(match.hasMatch())
    {
        QString valueStr = match.captured(1);
        bool conversionOk;
        double parsedValue = valueStr.toDouble(&conversionOk);
        if(!conversionOk)
            return false;
        _value = parsedValue;

        // Znajdź jednostkę - zwykle zaraz po wartości liczbowej
        int valuePos = data.indexOf(valueStr) + valueStr.length();

        // Pomiń białe znaki
        int unitStart = valuePos;
        while(unitStart < data.length() && data.at(unitStart).isSpace())
            unitStart++;

        // Pobierz znaki jednostki (do 3 znaków lub do białego znaku)
        if(unitStart < data.length())
        {
            int unitEnd = unitStart;
            int maxUnitChars = qMin(3, data.length() - unitStart);
            for(int i = 0; i < maxUnitChars; i++)
            {
                if(data.at(unitStart + i).isSpace() || data.at(unitStart + i) == '\r' || data.at(unitStart + i) == '\n')
                    break;
                unitEnd++;
            }
            _unitStr = data.mid(unitStart, unitEnd - unitStart);
            _unit = parseUnitFromString(_unitStr);
        }

        // Sprawdź stabilność (brak znaku zapytania oznacza stabilność)
        _stable = !data.contains("?");
        return true;
    }
    return false;
}

bool RadwagMeasure::parseStrictFormat(const QByteArray &data)
{
    // Format według dokumentacji:
    // 1 – 3 4 5 6 7 8 – 16 17 18 - 20 21 22
    // Rozkaz spacja znak_stabilności spacja znak Masa spacja jednostka CRLF

    if (data.size() < 14)
        return false;

    _stable = (data.at(4) == ' '); // spacja oznacza stabilny, '?' oznacza niestabilny

    // Pomijamy rozkaz i znaki kontrolne, szukamy wartości masowej
    // Typowo masa jest w pozycjach 8-16
    bool foundValue = false;

    for(int i = 6; i < data.size() - 5; i++)
    {
        if(isdigit(data.at(i)) || data.at(i) == '.')
        {
            // Szukaj końca liczby
            int j = i;
            while(j < data.size() && (isdigit(data.at(j)) || data.at(j) == '.'))
                j++;

            QString massStr = QString::fromLatin1(data.mid(i, j-i));
            bool conversionOk;
            double parsedValue = massStr.toDouble(&conversionOk);

            if (conversionOk)
            {
                _value = parsedValue;
                foundValue = true;

                // Szukaj jednostki po wartości liczbowej
                int unitPos = j;
                while(unitPos < data.size() && data.at(unitPos) == ' ')
                    unitPos++;

                // Pobierz jednostkę (do 3 znaków)
                int unitEnd = unitPos;
                while(unitEnd < qMin(unitPos + 3, data.size()) &&
                       data.at(unitEnd) != ' ' &&
                       data.at(unitEnd) != '\r' &&
                       data.at(unitEnd) != '\n')
                {
                    unitEnd++;
                }

                _unitStr = QString::fromLatin1(data.mid(unitPos, unitEnd - unitPos));
                _unit = parseUnitFromString(_unitStr);
                break;
            }
        }
    }

    return foundValue;
}

RadwagMeasure::Unit RadwagMeasure::parseUnitFromString(const QString &unitStr)
{
    QString unit = unitStr.trimmed().toLower();

    if(unit == "g")
        return Unit::G;
    if(unit == "kg")
        return Unit::KG;
    if(unit == "ct")
        return Unit::CT;
    if(unit == "lb")
        return Unit::LB;
    if(unit == "oz")
        return Unit::OZ;

    return Unit::Unknown;
}

// Gettery i settery
double RadwagMeasure::getValue(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(_value * factor) / factor;
}

void RadwagMeasure::setValue(double value)
{
    _value = value;
}

QString RadwagMeasure::getUnitString() const
{
    return _unitStr;
}

void RadwagMeasure::setUnitString(const QString &unitStr)
{
    _unitStr = unitStr;
    _unit = parseUnitFromString(unitStr);
}

RadwagMeasure::Unit RadwagMeasure::getUnit() const
{
    return _unit;
}

void RadwagMeasure::setUnit(Unit unit)
{
    _unit = unit;
}

bool RadwagMeasure::isStable() const
{
    return _stable;
}

void RadwagMeasure::setStable(bool stable)
{
    _stable = stable;
}


// #include "radwag_measure.h"

// RadwagMeasure::RadwagMeasure(const QByteArray& rawData) : DeviceData(rawData)
// {
//     parse();
// }
