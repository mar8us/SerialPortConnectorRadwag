#include "utils.h"

namespace utils
{

double getDouble(const QString &text)
{
    QString valueStr = text.trimmed();
    if(valueStr.isEmpty())
        return 0.0;

    valueStr = valueStr.replace(',', '.');
    bool conversionOk;
    double value = text.toDouble(&conversionOk);
    return conversionOk ? value : 0.0;
}

bool compareDouble(double a, double b, double epsilon)
{
    if(qAbs(a - b) <= epsilon)
        return true;

    return qAbs(a - b) <= epsilon * qMax(qAbs(a), qAbs(b));
}

SaturationMethod getSaturationMethodFromIndex(int index)
{
    switch (index)
    {
        case 0:
            return SaturationMethod::BoilingInWater;
        case 1:
            return SaturationMethod::VacuumMethod;
        case 2:
            return SaturationMethod::LongTermSoaking;
        default:
            return SaturationMethod::BoilingInWater;
    }
}

QString getSaturationMethodName(SaturationMethod method)
{
    switch (method)
    {
        case SaturationMethod::BoilingInWater:
            return "Gotowanie w wodzie";
        case SaturationMethod::VacuumMethod:
            return "Próżniowa";
        case SaturationMethod::LongTermSoaking:
            return "Długotrwałe nasączanie";
        default:
            return "Nieznana metoda";
    }
}

}
