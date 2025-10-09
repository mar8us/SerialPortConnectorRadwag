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
    QString hexData;
    for(char c : deviceData)
        hexData += QString("0x%1 ").arg((quint8)c, 2, 16, QLatin1Char('0'));

    qDebug() << "Otrzymano dane [bajty]:" << hexData;
    qDebug() << "Otrzymano dane [ASCII]:" << deviceData;

    m_buffer.append(deviceData);

    if(m_buffer.contains('\n'))
    {
        QList<QByteArray> lines = m_buffer.split('\n');

        for (int i = 0; i < lines.size() - 1; i++)
        {
            QByteArray line = lines[i];
            if(line.endsWith('\r'))
                line.chop(1); // Usuń \r

            processCompleteLine(line);
        }
        m_buffer = lines.last();
    }
    else if (m_buffer.contains('\r'))
    {
        QList<QByteArray> lines = m_buffer.split('\r');

        for(int i = 0; i < lines.size() - 1; i++)
            processCompleteLine(lines[i]);

        m_buffer = lines.last();
    }
    else if(m_buffer.size() > 10)
    // else if(m_buffer.size() > 20)
    {
        QRegularExpression rxUnitEnd("(g|kg|ct|lb|oz)\\s*$");
        QString bufferStr = QString::fromLatin1(m_buffer);

        QRegularExpressionMatch match = rxUnitEnd.match(bufferStr);
        if(match.hasMatch())
        {
            processCompleteLine(m_buffer);
            m_buffer.clear();
        }
    }

    qDebug() << "Bufor po przetworzeniu:" << m_buffer;
}

void RadwagScaleConnector::processCompleteLine(const QByteArray &lineData)
{
    qDebug() << "Przetwarzanie kompletnej linii:" << lineData;

    if(lineData.isEmpty())
        return;

    QString data = QString::fromUtf8(lineData).trimmed();
    qDebug() << "Po konwersji i przycinaniu:" << data;

    if(parseErrorCode(data) || parseCommandResponse(data))
        return;

    RadwagMeasure measure(lineData);
    if(measure.getValue() != 0.0 || !measure.getUnitString().isEmpty())
    {
        qDebug() << "Pomiar:" << measure.getValue() << measure.getUnitString() << (measure.isStable() ? "(stabilny)" : "(niestabilny)");
        emit radwagDataReady(measure);
    }
    else
        qDebug() << "Nie udało się sparsować linii jako pomiar";
}

bool RadwagScaleConnector::parseErrorCode(const QString &data)
{
    if (data == RadwagErrorCode::UNKNOWN_COMMAND)
    {
        qDebug() << "Błąd: Nieznana komenda";
        emit deviceError("Nieznana komenda");
        return true;
    }

    for(auto it = errorDescriptions.constBegin(); it != errorDescriptions.constEnd(); it++)
    {
        if(data.startsWith(it.key()))
        {
            qDebug() << "Błąd wagi:" << it.key() << "-" << it.value();
            emit deviceError(it.value());
            return true;
        }
    }
    return false;
}

bool RadwagScaleConnector::parseCommandResponse(const QString &data)
{
    if(data.length() < 3)
        return false;

    QString cmdResponse = data.mid(2, 2);
    QString cmdName = data.left(2);

    if(cmdResponse == RadwagResponseType::COMMAND_UNDERSTOOD)
    {
        qDebug() << "Komenda" << cmdName << "zrozumiana, rozpoczęto wykonywanie";
        return true;
    }
    else if(cmdResponse == RadwagResponseType::COMMAND_UNAVAILABLE)
    {
        qDebug() << "Komenda" << cmdName << "zrozumiana, ale w danym momencie niedostępna";
        emit deviceError("Komenda " + cmdName + " niedostępna");
        return true;
    }
    else if(cmdResponse == RadwagResponseType::COMMAND_MAX_RANGE_EXCEEDED)
    {
        qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu maksymalnego";
        emit deviceError("Przekroczony maksymalny zakres");
        return true;
    }
    else if(cmdResponse == RadwagResponseType::COMMAND_MIN_RANGE_EXCEEDED)
    {
        qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu minimalnego";
        emit deviceError("Przekroczony minimalny zakres");
        return true;
    }
    else if(cmdResponse == RadwagResponseType::COMMAND_ERROR)
    {
        qDebug() << "Wystąpił błąd przy wykonaniu komendy" << cmdName << "- przekroczony limit czasu";
        emit deviceError("Przekroczony limit czasu operacji");
        return true;
    }

    return false;
}
