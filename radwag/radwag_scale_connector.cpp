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
    if(deviceData.isEmpty())
        return;

    // Logowanie surowych danych (jeśli włączone)
    RadwagLogger::logRawData(deviceData, "logs/pomiar hex data.txt");

    // Dodaj nowe dane do bufora
    m_buffer.append(deviceData);

    const int MAX_BUFFER_SIZE = 50;
    if(m_buffer.size() > MAX_BUFFER_SIZE)
    {
        qWarning() << "RadwagScaleConnector: Bufor przepełniony (" << m_buffer.size() << "bajtów). Czyszczenie bufora.";
        qWarning() << "Zawartość bufora (HEX):" << m_buffer.toHex(' ');
        m_buffer.clear();
        emit deviceError("Przepełnienie bufora - odebrano nieprawidłowe dane z wagi");
        return;
    }

    while(true)
    {
        // Szukaj terminatora linii: \r lub \n
        int crPos = m_buffer.indexOf('\r');
        int lfPos = m_buffer.indexOf('\n');

        // Jeśli nie ma żadnego terminatora, czekaj na więcej danych
        if(crPos == -1 && lfPos == -1)
            break;

        // Znajdź pierwszy terminator
        int lineEndPos = -1;
        if(crPos != -1 && lfPos != -1)
            lineEndPos = qMin(crPos, lfPos);
        else if(crPos != -1)
            lineEndPos = crPos;
        else
            lineEndPos = lfPos;

        // Wyciągnij linię (bez terminatora)
        QByteArray line = m_buffer.left(lineEndPos);

        // Usuń linię i terminatory z bufora
        // Obsłuż zarówno \r, \n, jak i \r\n
        int bytesToRemove = lineEndPos + 1; // +1 dla pierwszego terminatora

        // Sprawdź czy następny bajt to też terminator (dla \r\n)
        if(bytesToRemove < m_buffer.size())
        {
            char nextChar = m_buffer.at(bytesToRemove);
            if((m_buffer.at(lineEndPos) == '\r' && nextChar == '\n') ||
                (m_buffer.at(lineEndPos) == '\n' && nextChar == '\r'))
            {
                bytesToRemove++; // Usuń także drugi terminator
            }
        }

        m_buffer.remove(0, bytesToRemove);

        // Przetwórz kompletną linię (jeśli nie jest pusta)
        if(!line.trimmed().isEmpty())
        {
            processCompleteLine(line);
        }
    }
}



void RadwagScaleConnector::processCompleteLine(const QByteArray &lineData)
{
    if(lineData.isEmpty())
        return;

    // Usuń podwojone znaki (problem z wagą: "TT  AA" -> "T  A", "EES" -> "ES")
    QByteArray cleanedData = removeDuplicateChars(lineData);

    // Konwersja do QString dla łatwiejszego przetwarzania
    QString data = QString::fromUtf8(cleanedData).trimmed();

    if(data.isEmpty())
        return;

    // Sprawdź typ odpowiedzi
    // 1. Kody błędów (ES, Er1-Er10)
    if(parseErrorCode(data))
        return;

    // 2. Odpowiedzi na komendy (_A, _I, _^, _v, _E)
    if(parseCommandResponse(data))
        return;

    // 3. Pomiary (SI, S, SU, SUI, C1, CU1)
    // Wszystko inne traktujemy jako pomiar
    RadwagMeasure measure(cleanedData);

    // Loguj sparsowany pomiar
    RadwagLogger::logParsedMeasure(measure, "logs/pomiar hex data.txt");

    // Emituj sygnał tylko jeśli parsowanie się powiodło i wartość jest sensowna
    if(measure.getValue() != 0.0 || !measure.getUnitString().isEmpty())
    {
        emit radwagDataReady(measure);
    }
}

QByteArray RadwagScaleConnector::removeDuplicateChars(const QByteArray &data)
{
    if(data.size() < 2)
        return data;

    QByteArray result;
    result.reserve(data.size());

    for(int i = 0; i < data.size(); i++)
    {
        char currentChar = data.at(i);

        // Dodaj znak do wyniku
        result.append(currentChar);

        // Usuń tylko podwojone LITERY (nie cyfry, nie znaki specjalne)
        // Problem wagi: "TT  AA" → "T  A", "EES" → "ES"
        // Ale NIE dotykaj cyfr: "3,3311" musi pozostać "3,3311"
        if(i + 1 < data.size() && data.at(i + 1) == currentChar)
        {
            // Sprawdź czy to litera (A-Z, a-z)
            bool isLetter = (currentChar >= 'A' && currentChar <= 'Z') ||
                           (currentChar >= 'a' && currentChar <= 'z');

            if(isLetter)
            {
                i++; // Pomiń duplikat litery
            }
        }
    }

    return result;
}

bool RadwagScaleConnector::parseErrorCode(const QString &data)
{
    // Sprawdź czy to kod błędu ES lub Er1-Er10
    if(data == "ES" || data.startsWith("ES"))
    {
        emit deviceError("Nieznana komenda (ES)");
        return true;
    }

    if(data.startsWith("Er"))
    {
        QString errorMsg = "Błąd wagi: " + data;

        // Opisy zgodnie z dokumentacją (sekcja 19)
        if(data.contains("Er1") || data.contains("Hi"))
            errorMsg = "Er1 (Hi) - Masa poza zakresem dopuszczalnym przy starcie wagi i przy kalibracji";
        else if(data.contains("Er2") || data.contains("nuLL"))
            errorMsg = "Er2 (nuLL) - Wartość z przetwornika A/C ≤ 0";
        else if(data.contains("Er3") || data.contains("FuL1"))
            errorMsg = "Er3 (FuL1) - Wartość z przetwornika A/C ≥ maksymalnego zakresu przetwornika";
        else if(data.contains("Er4") || data.contains("FuL2"))
            errorMsg = "Er4 (FuL2) - Przekroczony zakres maksymalny wagi";
        else if(data.contains("Er5") || data.contains("rout"))
            errorMsg = "Er5 (rout) - Wartość masy poza zakresem (przy kalibracji, wzorcu itd.)";
        else if(data.contains("Er7") || data.contains("tout"))
            errorMsg = "Er7 (tout) - Przekroczony czas operacji (np. zerowania, tarowania)";
        else if(data.contains("Er8") || data.contains("outr"))
            errorMsg = "Er8 (outr) - Wpisana wartość (z klawiatury) poza zakresem";
        else if(data.contains("Er9") || data.contains("Lock"))
            errorMsg = "Er9 (Lock) - Funkcja zablokowana (np. przez LFT)";
        else if(data.contains("Er10") || data.contains("cal"))
            errorMsg = "Er10 (cal) - Błąd kalibracji (zmiana masy lub niewłaściwa masa odważnika)";

        emit deviceError(errorMsg);
        return true;
    }

    return false;
}

bool RadwagScaleConnector::parseCommandResponse(const QString &data)
{
    // Format odpowiedzi: XX_A lub XX  A (ze spacjami zamiast podkreślenia)
    // Przykłady: "T  A", "SI A", "S  I", "Z  ^"

    // Sprawdź czy zawiera odpowiedź (ostatni znak A, I, ^, v, E, D)
    if(data.isEmpty() || data.length() < 2)
        return false;

    QChar lastChar = data.at(data.length() - 1);

    // Sprawdź czy to odpowiedź na komendę
    if(lastChar == 'A')
    {
        qDebug() << "Komenda zaakceptowana:" << data;
        // emit commandAccepted();
        return true;
    }

    if(lastChar == 'I')
    {
        emit deviceError("Komenda niedostępna w obecnym trybie: " + data);
        return true;
    }

    if(lastChar == '^')
    {
        emit deviceError("Przekroczony maksymalny zakres: " + data);
        return true;
    }

    if(lastChar == 'v')
    {
        emit deviceError("Przekroczony minimalny zakres: " + data);
        return true;
    }

    if(lastChar == 'E')
    {
        emit deviceError("Błąd wykonania komendy (timeout): " + data);
        return true;
    }

    if(lastChar == 'D')
    {
        qDebug() << "Komenda w trakcie wykonywania:" << data;
        // Odpowiedź 'D' - prawdopodobnie "Done" lub operacja w toku
        // Nie emitujemy błędu, tylko informację debug
        return true;
    }

    return false;
}
