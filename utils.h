#ifndef UTILS_H
#define UTILS_H
#include "radwag/measurement.h"
#include <QString>

namespace utils
{

double getDouble(const QString &text);
QString getSaturationMethodName(SaturationMethod method);
SaturationMethod getSaturationMethodFromIndex(int index);

}

#endif // UTILS_H
