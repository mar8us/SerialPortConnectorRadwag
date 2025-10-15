#ifndef MANUAL_MEASUREMENT_RECORD_H
#define MANUAL_MEASUREMENT_RECORD_H

#include <QDateTime>
#include <QString>
#include <QByteArray>

struct ManualMeasurementRecord
{
    QDateTime timestamp;
    double value;
    QString unit;
    bool isStable;
    QString label;
    QByteArray rawData;
};

#endif // MANUAL_MEASUREMENT_RECORD_H
