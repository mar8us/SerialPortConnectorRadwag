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

}
