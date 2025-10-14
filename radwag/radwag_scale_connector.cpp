#include "radwag_scale_connector.h"
#include <qdebug.h>
#include <qregularexpression.h>

RadwagScaleConnector::RadwagScaleConnector()
{

}

RadwagScaleConnector::~RadwagScaleConnector()
{

}

bool RadwagScaleConnector::sendTareCommand() const
{
    return sendCommand("T\r\n");
}

bool RadwagScaleConnector::sendZeroCommand() const
{
    return sendCommand("Z\r\n");
}

bool RadwagScaleConnector::sendImmediateWeightCommand() const
{
    return sendCommand("SI\r\n");
}

bool RadwagScaleConnector::sendStableWeightCommand() const
{
    return sendCommand("S\r\n");
}

bool RadwagScaleConnector::sendStableWeightCurrentUnitCommand() const
{
    return sendCommand("SU\r\n");
}

bool RadwagScaleConnector::sendImmediateWeightCurrentUnitCommand() const
{
    return sendCommand("SUI\r\n");
}

bool RadwagScaleConnector::startContinuousTransmissionBasicUnit() const
{
    return sendCommand("C1\r\n");
}

bool RadwagScaleConnector::stopContinuousTransmissionBasicUnit() const
{
    return sendCommand("C0\r\n");
}

bool RadwagScaleConnector::startContinuousTransmissionCurrentUnit() const
{
    return sendCommand("CU1\r\n");
}

bool RadwagScaleConnector::stopContinuousTransmissionCurrentUnit() const
{
    return sendCommand("CU0\r\n");
}



void RadwagScaleConnector::dataReceived(const QByteArray &deviceData)
{

}



void RadwagScaleConnector::processCompleteLine(const QByteArray &lineData)
{

}

bool RadwagScaleConnector::parseErrorCode(const QString &data)
{

    return false;
}

bool RadwagScaleConnector::parseCommandResponse(const QString &data)
{
    return false;
}
