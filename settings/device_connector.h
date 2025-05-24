#ifndef DEVICE_CONNECTOR_H
#define DEVICE_CONNECTOR_H

#include "device.h"
#include <QObject>
#include <QSerialPortInfo>
#include "../serialPort/serial_port.h"
#include <memory>

class DeviceData;

class DeviceConnector : public QObject
{
    Q_OBJECT
public:
    DeviceConnector();
    virtual ~DeviceConnector();

    virtual bool connectDevice(const QString &portName);
    virtual bool closeActiveConnection();
    virtual bool connectionIsActive();

    std::shared_ptr<const Device> getActiveDevice();
    void setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice);

    bool sendCommand(const QByteArray &command);

protected:
    std::shared_ptr<const Device> activeDevice;
    std::unique_ptr<SerialPort> serialPort;
    QByteArray m_buffer;

protected slots:
    virtual void dataReceived(const QByteArray &deviceData) = 0;

signals:
    void connectionResult(bool connected);
    void dataReady(const QByteArray &data);
    void deviceError(const QString &errorMessage);
};


// #endif // RADWAG_SCALE_CONNECTOR_H

#endif // DEVICE_CONNECTOR_H

// #ifndef DEVICE_CONNECTOR_H
// #define DEVICE_CONNECTOR_H

// #include "device.h"
// #include "QSerialPortInfo"
// #include "../serialPort/serial_port.h"
// #include "../radwag/radwag_measure.h"
// #include <QMap>

// namespace RadwagErrorCode {
// const QString UNKNOWN_COMMAND = "ES";
// const QString ERR1 = "Er1";
// const QString ERR2 = "Er2";
// const QString ERR3 = "Er3";
// const QString ERR4 = "Er4";
// const QString ERR5 = "Er5";
// const QString ERR7 = "Er7";
// const QString ERR8 = "Er8";
// const QString ERR9 = "Er9";
// const QString ERR10 = "Er10";
// }



// namespace RadwagResponseType
// {
// const QString COMMAND_UNDERSTOOD = "_A";
// const QString COMMAND_UNAVAILABLE = "_I";
// const QString COMMAND_MAX_RANGE_EXCEEDED = "_^";
// const QString COMMAND_MIN_RANGE_EXCEEDED = "_v";
// const QString COMMAND_ERROR = "_E";
// }

// class DeviceConnector : public QObject
// {
//     Q_OBJECT
// public:
//     DeviceConnector();

//     bool connectDevice(const QString &portName);
//     bool closeActiveConnection();
//     bool connectionIsActive();

//     std::shared_ptr<const Device> getActiveDevice();
//     void setActiveDevice(const std::shared_ptr<const Device>& newActiveDevice);
//     QStringList getAvaiablePorts();
//     void sendImmediateWeightCommand2();

//     // Podstawowe komendy
//     void sendImmediateWeightCommand();
//     void sendTareCommand();
//     void sendZeroCommand();

//     // Komendy do odbierania wyników
//     void sendStableWeightCommand();
//     void sendStableWeightCurrentUnitCommand();
//     void sendImmediateWeightCurrentUnitCommand();

//     // Komendy do transmisji ciągłej
//     void startContinuousTransmissionBasicUnit();
//     void stopContinuousTransmissionBasicUnit();
//     void startContinuousTransmissionCurrentUnit();
//     void stopContinuousTransmissionCurrentUnit();

// private:
//     bool sendCommand(const QByteArray &command);

//     QByteArray m_buffer;
//     void processCompleteLine(const QByteArray &lineData);

//     std::shared_ptr<const Device> activeDevice;
//     std::unique_ptr<SerialPort> serialPort;

//     const QMap<QString, QString> errorDescriptions =
//     {
//         {RadwagErrorCode::UNKNOWN_COMMAND, "Nieznana komenda"},
//         {RadwagErrorCode::ERR1, "Hi - masa poza zakresem dopuszczalnym przy starcie wagi i przy kalibracji"},
//         {RadwagErrorCode::ERR2, "nuLL - wartość z przetwornika A/C ≤ 0"},
//         {RadwagErrorCode::ERR3, "FuL1 - wartość z przetwornika A/C ≥ maksymalnego zakresu przetwornika"},
//         {RadwagErrorCode::ERR4, "FuL2 - przekroczony zakres maksymalny wagi"},
//         {RadwagErrorCode::ERR5, "rout - wartość masy poza zakresem"},
//         {RadwagErrorCode::ERR7, "tout - przekroczony czas operacji (np. zerowania, tarowania)"},
//         {RadwagErrorCode::ERR8, "outr - wpisana wartość (z klawiatury) poza zakresem"},
//         {RadwagErrorCode::ERR9, "Lock - funkcja zablokowana (np. przez LFT)"},
//         {RadwagErrorCode::ERR10, "cal - błąd kalibracji"}
//     };

// private slots:
//     void dataReceived(const QByteArray &deviceData);

// signals:
//     void connectionResult(bool connected);

//     void radwagDataReady(const RadwagMeasure &data);
// };

// #endif // DEVICE_CONNECTOR_H
