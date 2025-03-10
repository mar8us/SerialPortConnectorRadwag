#ifndef RADWAG_MEASURE_H
#define RADWAG_MEASURE_H
#include "../settings/device_data.h"
#include <QString>

enum class Unit
{
    Unknown,
    Milligrams,
    Grams,
    Kilograms,
};

class RadwagMeasure : public DeviceData
{
public:
    RadwagMeasure(const QByteArray& rawData);

    bool parse() override
    {
        _order = rawData.mid(0, 3);
        _stable = rawData.at(3) == 'S';
        _value = parseValueFromRawData();
        _unit = parseUnitFromRawData();

        return true;
    }

    QString getOrder() const
    {
        return _order;
    }

    bool isStable() const
    {
        return _stable;
    }

    double getValue() const
    {
        return _value;
    }

    Unit getUnit() const
    {
        return _unit;
    }


    bool convertTo(Unit targetUnit)
    {

        return true;
    }

private:
    QString _order;
    bool _stable = false;
    double _value = 0.0;
    Unit _unit = Unit::Unknown;

    double parseValueFromRawData()
    {
        return 0.0;
    }

    Unit parseUnitFromRawData()
    {
        return Unit::Unknown;
    }
};
#endif // RADWAG_MEASURE_H
