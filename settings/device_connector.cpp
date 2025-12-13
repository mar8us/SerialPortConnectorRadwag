#include "device_connector.h"
#include <qregularexpression.h>

DeviceConnector::DeviceConnector()
    : serialPort(std::make_unique<SerialPort>())
{

}

DeviceConnector::~DeviceConnector()
{

}

bool DeviceConnector::connectDevice(const QString &portName) const
{
    if(!activeDevice.get())
        return false;

    disconnect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);

    if(!serialPort->connect(portName, activeDevice->getBaudRate(), activeDevice->getDataBits(), activeDevice->getParity(), activeDevice->getStopBits()))
    {
        connectionResult(false);
        return false;
    }

    connect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);
    emit connectionResult(true);
    return true;
}

bool DeviceConnector::closeActiveConnection()
{
    if(!serialPort->isOpen())
    {
        emit connectionResult(false);
        return false;
    }

    disconnect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);

    m_buffer.clear();

    bool connectionClosed = serialPort->closeConnection();
    activeDevice = nullptr;
    emit connectionResult(!connectionClosed);
    return !connectionClosed;
}

bool DeviceConnector::connectionIsActive() const
{
    return serialPort->isOpen();
}

std::shared_ptr<const Device> DeviceConnector::getActiveDevice() const
{
    return activeDevice;
}

void DeviceConnector::setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice)
{
    activeDevice = newActiveDevice;
}

bool DeviceConnector::sendCommand(const QByteArray &command) const
{
    if(!connectionIsActive())
        return false;
    return serialPort->write(command) == -1 ? false : true;
}

int DeviceConnector::clearData()
{
    int size = m_buffer.size();
    m_buffer.clear();
    return size;
}

void DeviceConnector::dataReceived(const QByteArray &deviceData)
{
    emit dataReady(deviceData);
}


// #include "device_connector.h"

// DeviceConnector::DeviceConnector()
//     : serialPort(std::make_unique<SerialPort>())
// {

// }
// bool DeviceConnector::connectDevice(const QString &portName)
// {
//     if(!activeDevice.get())
//         return false;

//     if(!serialPort->connect(portName, activeDevice->getBaudRate(), activeDevice->getDataBits(), activeDevice->getParity(), activeDevice->getStopBits()))
//     {
//         connectionResult(false);
//         return false;
//     }

//     connect(serialPort.get(), &SerialPort::dataRecevied, this, &DeviceConnector::dataReceived);
//     emit connectionResult(true);
//     return true;
// }

// bool DeviceConnector::closeActiveConnection()
// {
//     if(!serialPort->isOpen())
//     {
//         emit connectionResult(false);
//         return false;
//     }
//     bool connectionClosed = serialPort->closeConnection();
//     emit connectionResult(!connectionClosed);
//     return !connectionClosed;
// }

// bool DeviceConnector::connectionIsActive()
// {
//     return serialPort->isOpen();
// }

// std::shared_ptr<const Device> DeviceConnector::getActiveDevice()
// {
//     return activeDevice;
// }

// void DeviceConnector::setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice)
// {
//     activeDevice = newActiveDevice;
// }

// QStringList DeviceConnector::getAvailablePorts()
// {
//     QStringList ports;
//     foreach(auto&port, QSerialPortInfo::availablePorts())
//         ports.append(port.portName());
//     return ports;
// }

// #include <qmessagebox.h>
// // void DeviceConnector::dataReceived(const QByteArray &deviceData)
// // {

// //     // Dodaj logowanie surowych danych
// //     QString hexData;
// //     for (char c : deviceData) {
// //         hexData += QString("0x%1 ").arg((quint8)c, 2, 16, QLatin1Char('0'));
// //     }
// //     qDebug() << "Otrzymano dane [bajty]:" << hexData;
// //     qDebug() << "Otrzymano dane [ASCII]:" << deviceData;

// //     QString data = QString::fromUtf8(deviceData).trimmed();
// //     qDebug() << "Po konwersji i przycinaniu:" << data;

// //     // QString data = QString::fromUtf8(deviceData).trimmed();

// //     if(data == RadwagErrorCode::UNKNOWN_COMMAND)
// //     {
// //         qDebug() << "Błąd: Nieznana komenda";
// //         return;
// //     }

// //     for(auto it = errorDescriptions.constBegin(); it != errorDescriptions.constEnd(); ++it)
// //     {
// //         if(data.startsWith(it.key()))
// //         {
// //             qDebug() << "Błąd wagi:" << it.key() << "-" << it.value();
// //             return;
// //         }
// //     }

// //     if(data.length() >= 3)
// //     {
// //         QString cmdResponse = data.mid(2, 2);
// //         QString cmdName = data.left(2);

// //         if(cmdResponse == RadwagResponseType::COMMAND_UNDERSTOOD)
// //         {
// //             qDebug() << "Komenda" << cmdName << "zrozumiana, rozpoczęto wykonywanie";
// //             return;
// //         }
// //         else if(cmdResponse == RadwagResponseType::COMMAND_UNAVAILABLE)
// //         {
// //             qDebug() << "Komenda" << cmdName << "zrozumiana, ale w danym momencie niedostępna";
// //             return;
// //         }
// //         else if(cmdResponse == RadwagResponseType::COMMAND_MAX_RANGE_EXCEEDED)
// //         {
// //             qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu maksymalnego";
// //             return;
// //         }
// //         else if(cmdResponse == RadwagResponseType::COMMAND_MIN_RANGE_EXCEEDED)
// //         {
// //             qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu minimalnego";
// //             return;
// //         }
// //         else if(cmdResponse == RadwagResponseType::COMMAND_ERROR)
// //         {
// //             qDebug() << "Wystąpił błąd przy wykonaniu komendy" << cmdName << "- przekroczony limit czasu";
// //             return;
// //         }
// //     }

// //     RadwagMeasure radwagMeasure(deviceData);
// //     if(radwagMeasure.getValue())
// //     {
// //         qDebug() << (QString::number(radwagMeasure.getValue()) + " AAAAAAAAAAAAAAAAAA" +  radwagMeasure.getUnit());
// //         emit radwagDataReady(radwagMeasure);
// //     }
// //     // qDebug() << (radwagMeasure.isStable() ? "Stabilny " : "Niestabilny ") + QString::number(radwagMeasure.getValue()) + " " +  radwagMeasure.getUnit();
// //     // emit radwagDataReady(radwagMeasure);
// //     qDebug() << "END ---" << "----END";
// // }



// void DeviceConnector::dataReceived(const QByteArray &deviceData)
// {
//     // Dodaj logowanie surowych danych
//     QString hexData;
//     for(char c : deviceData)
//         hexData += QString("0x%1 ").arg((quint8)c, 2, 16, QLatin1Char('0'));

//     qDebug() << "Otrzymano dane [bajty]:" << hexData;
//     qDebug() << "Otrzymano dane [ASCII]:" << deviceData;

//     // Dodaj dane do bufora
//     m_buffer.append(deviceData);

//     // Sprawdź, czy w buforze jest kompletna linia
//     if (m_buffer.contains('\n')) {
//         QList<QByteArray> lines = m_buffer.split('\n');

//         // Przetwarzaj wszystkie pełne linie
//         for (int i = 0; i < lines.size() - 1; ++i) {
//             QByteArray line = lines[i];
//             if (line.endsWith('\r'))
//                 line.chop(1); // Usuń \r

//             processCompleteLine(line);
//         }

//         // Zachowaj ostatnią (potencjalnie niekompletną) linię
//         m_buffer = lines.last();
//     }
//     RadwagMeasure radwagMeasure(deviceData);
//     emit radwagDataReady(radwagMeasure);
//     qDebug() << "END ---" << "----END";
// }

// // Dodaj nową metodę do przetwarzania kompletnych linii
// void DeviceConnector::processCompleteLine(const QByteArray &lineData)
// {
//     qDebug() << "Przetwarzanie kompletnej linii:" << lineData;

//     QString data = QString::fromUtf8(lineData).trimmed();
//     qDebug() << "Po konwersji i przycinaniu (kompletna linia):" << data;

//     if(data == RadwagErrorCode::UNKNOWN_COMMAND)
//     {
//         qDebug() << "Błąd: Nieznana komenda";
//         return;
//     }

//     for(auto it = errorDescriptions.constBegin(); it != errorDescriptions.constEnd(); ++it)
//     {
//         if(data.startsWith(it.key()))
//         {
//             qDebug() << "Błąd wagi:" << it.key() << "-" << it.value();
//             return;
//         }
//     }

//     if(data.length() >= 3)
//     {
//         QString cmdResponse = data.mid(2, 2);
//         QString cmdName = data.left(2);
//         if(cmdResponse == RadwagResponseType::COMMAND_UNDERSTOOD)
//         {
//             qDebug() << "Komenda" << cmdName << "zrozumiana, rozpoczęto wykonywanie";
//             return;
//         }
//         else if(cmdResponse == RadwagResponseType::COMMAND_UNAVAILABLE)
//         {
//             qDebug() << "Komenda" << cmdName << "zrozumiana, ale w danym momencie niedostępna";
//             return;
//         }
//         else if(cmdResponse == RadwagResponseType::COMMAND_MAX_RANGE_EXCEEDED)
//         {
//             qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu maksymalnego";
//             return;
//         }
//         else if(cmdResponse == RadwagResponseType::COMMAND_MIN_RANGE_EXCEEDED)
//         {
//             qDebug() << "Komenda" << cmdName << "zrozumiana, ale wystąpiło przekroczenie zakresu minimalnego";
//             return;
//         }
//         else if(cmdResponse == RadwagResponseType::COMMAND_ERROR)
//         {
//             qDebug() << "Wystąpił błąd przy wykonaniu komendy" << cmdName << "- przekroczony limit czasu";
//             return;
//         }
//     }

//     // Przetwarzanie pomiaru tylko dla niepustych danych
//     if (!lineData.isEmpty()) {
//         RadwagMeasure radwagMeasure(lineData);
//         if(radwagMeasure.getValue() != 0.0) {  // Zakładając, że 0.0 to wartość domyślna/nieprawidłowa
//             qDebug() << (QString::number(radwagMeasure.getValue()) + " " +  radwagMeasure.getUnit());
//             emit radwagDataReady(radwagMeasure);
//         }
//     }
// }

// void DeviceConnector::sendImmediateWeightCommand2()
// {
//     QByteArray command = "Z\r\n";  // SI + CR (carriage return) + LF (line feed)
//     qint64 bytesWritten = serialPort->write(command);

//     if (bytesWritten == -1) {
//         qDebug() << "Błąd: Nie można wysłać komendy do wagi";
//     } else {
//         qDebug() << "Wysłano komendę SI. Zapisano bajtów:" << bytesWritten;
//     }
// }

// bool DeviceConnector::sendCommand(const QByteArray &command)
// {
//     if(!connectionIsActive())
//         return false;

//     qint64 bytesWritten = serialPort->write(command);
//     if (bytesWritten == -1)
//     {
//         qDebug() << "Błąd: Nie można wysłać komendy do wagi";
//         return false;
//     }
//     else
//     {
//         qDebug() << "Wysłano komendę: " << command << ". Zapisano bajtów:" << bytesWritten;
//         return true;
//     }
// }

// void DeviceConnector::sendTareCommand()
// {
//     sendCommand("T\r\n");
// }

// void DeviceConnector::sendZeroCommand()
// {
//     sendCommand("Z\r\n");
// }

// void DeviceConnector::sendImmediateWeightCommand()
// {
//     sendCommand("SI\r\n");
// }

// void DeviceConnector::sendStableWeightCommand()
// {
//     sendCommand("S\r\n");
// }

// void DeviceConnector::sendStableWeightCurrentUnitCommand()
// {
//     sendCommand("SU\r\n");
// }

// void DeviceConnector::sendImmediateWeightCurrentUnitCommand()
// {
//     sendCommand("SUI\r\n");
// }

// void DeviceConnector::startContinuousTransmissionBasicUnit()
// {
//     sendCommand("C1\r\n");
// }

// void DeviceConnector::stopContinuousTransmissionBasicUnit()
// {
//     sendCommand("C0\r\n");
// }

// void DeviceConnector::startContinuousTransmissionCurrentUnit()
// {
//     sendCommand("CU1\r\n");
// }

// void DeviceConnector::stopContinuousTransmissionCurrentUnit()
// {
//     sendCommand("CU0\r\n");
// }

