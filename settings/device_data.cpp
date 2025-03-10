#include "device_data.h"

DeviceData::DeviceData(const QByteArray& rawData)
    : rawData(rawData)
{

}

const QByteArray& DeviceData::getData() const
{
    return rawData;
}
