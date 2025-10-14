#include "radwag_measure.h"
#include <QRegularExpression>
#include <qdatetime.h>
#include <qobject.h>
#include <qserialport.h>

#include <QFile>

#include <QFileInfo>

#include <QDir>

RadwagMeasure::RadwagMeasure(const QByteArray& rawData)
    : DeviceData(rawData), _value(0.0), _unitStr(""), _unit(Unit::Unknown), _stable(false)
{
    parse();
}

RadwagMeasure::~RadwagMeasure()
{

}

bool RadwagMeasure::parse()
{
    if(rawData.isEmpty())
        return false;

    // Użyj UTF-8 dla spójności
    QString data = QString::fromUtf8(rawData).trimmed();

    if(parseFlexibly(data))
        return true;
    return true;
    //return parseStrictFormat(rawData);
}

bool RadwagMeasure::parseFlexibly(const QString &data)
{
    // Format Radwag: "4,321[2]g" gdzie [2] to dodatkowa cyfra po przecinku
    // Regex: wartość (z przecinkiem lub kropką) + opcjonalna cyfra w [nawiasach]
    QRegularExpression rxRadwag("([+-]?\\d+[,\\.]\\d+)(?:\\[(\\d)\\])?");
    QRegularExpressionMatch match = rxRadwag.match(data);

    if(match.hasMatch())
    {
        QString valueStr = match.captured(1);        // np. "4,321"
        QString extraDigit = match.captured(2);      // np. "2" (może być pusty)

        // Zamień przecinek na kropkę dla toDouble()
        valueStr.replace(',', '.');

        // Jeśli jest dodatkowa cyfra w [nawiasach], dołącz ją do wartości
        if(!extraDigit.isEmpty())
        {
            valueStr += extraDigit;  // "4.321" + "2" = "4.3212"
        }

        bool conversionOk;
        double parsedValue = valueStr.toDouble(&conversionOk);

        if(!conversionOk)
            return false;

        _value = parsedValue;

        // Znajdź jednostkę - może być po nawiasach lub bezpośrednio po wartości
        // Szukaj od pozycji po całym dopasowaniu (wartość + ewentualne [n])
        int searchStart = data.indexOf(match.captured(0)) + match.captured(0).length();

        // Pomiń białe znaki
        int unitStart = searchStart;
        while(unitStart < data.length() && data.at(unitStart).isSpace())
            unitStart++;

        // Pobierz znaki jednostki (do 3 znaków lub do białego znaku)
        if(unitStart < data.length())
        {
            int unitEnd = unitStart;
            int maxUnitChars = qMin(3, data.length() - unitStart);
            for(int i = 0; i < maxUnitChars; i++)
            {
                if(unitStart + i >= data.length())
                    break;

                QChar ch = data.at(unitStart + i);
                if(ch.isSpace() || ch == '\r' || ch == '\n')
                    break;

                unitEnd++;
            }

            _unitStr = data.mid(unitStart, unitEnd - unitStart).trimmed();
            _unit = parseUnitFromString(_unitStr);
        }

        // Sprawdź stabilność (brak znaku zapytania oznacza stabilność)
        _stable = !data.contains("?") && !data.contains("~");

        return true;
    }

    return false;
}

bool RadwagMeasure::parseFlexibly2(const QString &data)
{
    qDebug() << "=== PARSOWANIE RADWAG ===";
    qDebug() << "Dane wejściowe:" << data;

    // Format Radwag: "3,321[4]g" gdzie [4] to dodatkowa cyfra po przecinku
    // Regex: wartość + opcjonalna cyfra w nawiasach + jednostka
    QRegularExpression rxRadwag("([+-]?\\d+[\\.,]\\d+)\\[(\\d)\\]\\s*(\\w*)");
    QRegularExpressionMatch match = rxRadwag.match(data);

    if(match.hasMatch())
    {
        QString valueStr = match.captured(1);        // "3,321"
        QString extraDigit = match.captured(2);      // "4"
        QString unitStr = match.captured(3);         // "g"

        qDebug() << "Dopasowano format Radwag:";
        qDebug() << "  Wartość podstawowa:" << valueStr;
        qDebug() << "  Dodatkowa cyfra [n]:" << extraDigit;
        qDebug() << "  Jednostka:" << unitStr;

        // Zamień przecinek na kropkę
        valueStr.replace(',', '.');

        // POŁĄCZ wartość z dodatkową cyfrą: "3.321" + "4" = "3.3214"
        QString fullValueStr = valueStr + extraDigit;

        qDebug() << "  Pełna wartość (string):" << fullValueStr;

        bool conversionOk;
        double parsedValue = fullValueStr.toDouble(&conversionOk);

        if(!conversionOk)
        {
            qDebug() << "BŁĄD: Nie udało się skonwertować" << fullValueStr << "na double";
            return false;
        }

        _value = parsedValue;
        _unitStr = unitStr;
        _unit = parseUnitFromString(_unitStr);
        _stable = !data.contains("?") && !data.contains("~");

        qDebug() << "=== WYNIK ===";
        qDebug() << "Wartość:" << QString::number(_value, 'f', 4);
        qDebug() << "Jednostka:" << _unitStr;
        qDebug() << "Stabilność:" << (_stable ? "TAK" : "NIE");

        return true;
    }

    // Fallback - format bez dodatkowej cyfry (starsze wagi lub inne tryby)
    QRegularExpression rxSimple("([+-]?\\d+[\\.,]\\d+)\\s*(\\w*)");
    match = rxSimple.match(data);

    if(match.hasMatch())
    {
        qDebug() << "Dopasowano prosty format (bez [n]):";
        QString valueStr = match.captured(1);
        QString unitStr = match.captured(2);

        valueStr.replace(',', '.');

        qDebug() << "  Wartość:" << valueStr;
        qDebug() << "  Jednostka:" << unitStr;

        bool conversionOk;
        double parsedValue = valueStr.toDouble(&conversionOk);

        if(!conversionOk)
        {
            qDebug() << "BŁĄD: Konwersja nie powiodła się";
            return false;
        }

        _value = parsedValue;
        _unitStr = unitStr;
        _unit = parseUnitFromString(_unitStr);
        _stable = !data.contains("?");

        qDebug() << "=== WYNIK ===";
        qDebug() << "Wartość:" << QString::number(_value, 'f', 4);
        qDebug() << "Jednostka:" << _unitStr;

        return true;
    }

    qDebug() << "BŁĄD: Nie udało się sparsować danych";
    return false;
}


RadwagMeasure::Unit RadwagMeasure::parseUnitFromString(const QString &unitStr)
{
    QString unit = unitStr.trimmed().toLower();

    if(unit == "g")
        return Unit::G;
    if(unit == "kg")
        return Unit::KG;
    if(unit == "ct")
        return Unit::CT;
    if(unit == "lb")
        return Unit::LB;
    if(unit == "oz")
        return Unit::OZ;

    return Unit::Unknown;
}

// Gettery i settery
double RadwagMeasure::getValue(int precision) const
{
    //double factor = std::pow(10.0, precision);
    return _value;//std::round(_value * factor) / factor;
}

void RadwagMeasure::setValue(double value)
{
    _value = value;
}

QString RadwagMeasure::getUnitString() const
{
    return _unitStr;
}

void RadwagMeasure::setUnitString(const QString &unitStr)
{
    _unitStr = unitStr;
    _unit = parseUnitFromString(unitStr);
}

RadwagMeasure::Unit RadwagMeasure::getUnit() const
{
    return _unit;
}

void RadwagMeasure::setUnit(Unit unit)
{
    _unit = unit;
}

bool RadwagMeasure::isStable() const
{
    return _stable;
}

void RadwagMeasure::setStable(bool stable)
{
    _stable = stable;
}


namespace RadwagLogger
{
// Statyczne zmienne wewnętrzne
static QString s_defaultLogPath;
static bool s_loggingEnabled = false;

void setDefaultLogPath(const QString& path)
{
    s_defaultLogPath = path;

    // Utwórz katalog jeśli nie istnieje
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();
    if(!dir.exists())
    {
        dir.mkpath(".");
        qDebug() << "Utworzono katalog dla logów:" << dir.absolutePath();
    }
}

QString getDefaultLogPath()
{
    if(s_defaultLogPath.isEmpty())
    {
        QString logDir = QDir::currentPath() + "/logs";
        QDir().mkpath(logDir);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        s_defaultLogPath = logDir + "/radwag_" + timestamp + ".log";
    }
    return s_defaultLogPath;
}

void enableLogging(bool enable)
{
    s_loggingEnabled = enable;
    if(enable)
        qDebug() << "Logowanie Radwag włączone. Plik:" << getDefaultLogPath();
    else
        qDebug() << "Logowanie Radwag wyłączone.";
}

bool isLoggingEnabled()
{
    return s_loggingEnabled;
}


void logRawData(const QByteArray& rawData, const QString& filePath)
{
    if(!s_loggingEnabled)
        return;

    QString logPath = filePath.isEmpty() ? getDefaultLogPath() : filePath;

    QFile logFile(logPath);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "Nie można otworzyć pliku logowania:" << logPath;
        qWarning() << "Błąd:" << logFile.errorString();
        return;
    }

    QTextStream out(&logFile);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    out << "========================================\n";
    out << "[RAW DATA RECEIVED]\n";
    out << "TIMESTAMP: " << timestamp << "\n";
    out << "----------------------------------------\n";

    // RAW DATA - HEX
    out << "HEX: ";
    for(int i = 0; i < rawData.size(); i++)
    {
        unsigned char byte = (unsigned char)rawData[i];
        out << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
        if(i < rawData.size() - 1)
            out << " ";
    }
    out << "\n";

    // RAW DATA - ASCII/readable
    out << "ASCII: ";
    for(int i = 0; i < rawData.size(); i++)
    {
        unsigned char c = (unsigned char)rawData[i];
        if(c >= 32 && c < 127)
            out << QChar(c);
        else
            out << QString("[%1]").arg(c, 2, 16, QChar('0')).toUpper();
    }
    out << "\n";

    out << "SIZE: " << rawData.size() << " bytes\n";
    out << "========================================\n\n";

    logFile.close();
}

void logParsedMeasure(const RadwagMeasure& measure, const QString& filePath)
{
    if(!s_loggingEnabled)
        return;

    QString logPath = filePath.isEmpty() ? getDefaultLogPath() : filePath;

    QFile logFile(logPath);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "Nie można otworzyć pliku logowania:" << logPath;
        qWarning() << "Błąd:" << logFile.errorString();
        return;
    }

    QTextStream out(&logFile);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    out << "========================================\n";
    out << "[PARSED MEASUREMENT]\n";
    out << "TIMESTAMP: " << timestamp << "\n";
    out << "----------------------------------------\n";

    // RAW DATA z pomiaru
    const QByteArray& rawData = measure.getData();
    out << "RAW HEX: ";
    for(int i = 0; i < rawData.size(); i++)
    {
        unsigned char byte = (unsigned char)rawData[i];
        out << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
        if(i < rawData.size() - 1)
            out << " ";
    }
    out << "\n";

    out << "RAW ASCII: ";
    for(int i = 0; i < rawData.size(); i++)
    {
        unsigned char c = (unsigned char)rawData[i];
        if(c >= 32 && c < 127)
            out << QChar(c);
        else
            out << QString("[%1]").arg(c, 2, 16, QChar('0')).toUpper();
    }
    out << "\n";

    out << "----------------------------------------\n";

    // PARSED VALUES
    out << "VALUE: " << QString::number(measure.getValue(), 'f', 4) << "\n";
    out << "VALUE (scientific): " << QString::number(measure.getValue(), 'e', 10) << "\n";
    out << "UNIT: " << measure.getUnitString() << "\n";
    out << "STABLE: " << (measure.isStable() ? "YES" : "NO") << "\n";
    out << "========================================\n\n";

    logFile.close();
}
}

// #include "radwag_measure.h"

// RadwagMeasure::RadwagMeasure(const QByteArray& rawData) : DeviceData(rawData)
// {
//     parse();
// }
