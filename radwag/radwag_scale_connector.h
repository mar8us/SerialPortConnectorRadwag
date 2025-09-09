#ifndef RADWAG_SCALE_CONNECTOR_H
#define RADWAG_SCALE_CONNECTOR_H

#include "../settings/device_connector.h"
#include "../radwag/radwag_measure.h"
#include <QMap>
#include <qobjectdefs.h>

namespace RadwagErrorCode
{
const QString UNKNOWN_COMMAND = "ES";
const QString ERR1 = "Er1";
const QString ERR2 = "Er2";
const QString ERR3 = "Er3";
const QString ERR4 = "Er4";
const QString ERR5 = "Er5";
const QString ERR7 = "Er7";
const QString ERR8 = "Er8";
const QString ERR9 = "Er9";
const QString ERR10 = "Er10";
}

namespace RadwagResponseType
{
const QString COMMAND_UNDERSTOOD = "_A";
const QString COMMAND_UNAVAILABLE = "_I";
const QString COMMAND_MAX_RANGE_EXCEEDED = "_^";
const QString COMMAND_MIN_RANGE_EXCEEDED = "_v";
const QString COMMAND_ERROR = "_E";
}

class RadwagScaleConnector : public DeviceConnector
{
    Q_OBJECT
public:
    explicit RadwagScaleConnector();
    virtual ~RadwagScaleConnector();

    void sendTareCommand() const;
    void sendZeroCommand() const;
    void sendImmediateWeightCommand() const;
    void sendStableWeightCommand() const;
    void sendStableWeightCurrentUnitCommand() const;
    void sendImmediateWeightCurrentUnitCommand() const;
    void startContinuousTransmissionBasicUnit() const;
    void stopContinuousTransmissionBasicUnit() const;
    void startContinuousTransmissionCurrentUnit() const;
    void stopContinuousTransmissionCurrentUnit() const;

protected:
    void processCompleteLine(const QByteArray &lineData);

protected slots:
    virtual void dataReceived(const QByteArray &deviceData) override;

private:
    const QMap<QString, QString> errorDescriptions;
    bool parseCommandResponse(const QString &data);
    bool parseErrorCode(const QString &data);

signals:
    void radwagDataReady(const RadwagMeasure &data);
};

#endif // RADWAG_SCALE_CONNECTOR_H
