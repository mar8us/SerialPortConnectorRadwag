#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include <QByteArray>

class DeviceData
{
public:
    DeviceData(const QByteArray& rawData);
    virtual ~DeviceData() = default;

    virtual bool parse() = 0;
    const QByteArray& getData() const;

protected:
    QByteArray rawData;
};

#endif // DEVICE_DATA_H
