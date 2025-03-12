#include "radwag_measure.h"

RadwagMeasure::RadwagMeasure(const QByteArray& rawData) : DeviceData(rawData)
{
    parse();
}
