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

    return parseFlexibly(rawData);
}

bool RadwagMeasure::parseFlexibly(const QByteArray &data)
{
    // Reset wartości
    _value = 0.0;
    _unitStr.clear();
    _unit = Unit::Unknown;
    _stable = true;  // Domyślnie stabilny, dopóki nie znajdziemy ?, ^, v

    // Format Radwag wg dokumentacji:
    // [Rozkaz][spacja][znak stabilności][spacja][znak][Masa][spacja][jednostka][CR][LF]
    //
    // Przykłady:
    // "SI       9.988[5] g"     - SI + spacja + stabilność + spacja + znak + masa + jednostka
    // "SUI   -   4.333[5] g"    - SUI + spacja + stabilność + spacja + minus + masa + jednostka
    // "      4.333[5] g"        - bez rozkazu, same spacje

    int pos = 0;
    int len = data.size();

    // KROK 1: Parsuj ROZKAZ (SI, SUI, SU, S, C1, CU1)
    // Rozkazy mogą mieć 1-3 znaki
    if(data.startsWith("SUI"))
    {
        pos = 3;
    }
    else if(data.startsWith("SI"))
    {
        pos = 2;
    }
    else if(data.startsWith("SU"))
    {
        pos = 2;
    }
    else if(data.startsWith("C1") || data.startsWith("CU"))
    {
        pos = 2;
        if(pos < len && data[pos] == '1')
            pos = 3;  // CU1
    }
    else if(data.startsWith("S"))
    {
        pos = 1;
    }
    // Jeśli nie ma rozkazu, pos = 0

    // KROK 2: Pomiń SPACJĘ po rozkazie
    if(pos < len && data[pos] == ' ')
        pos++;

    // KROK 3: Sprawdź ZNAK STABILNOŚCI
    // [spacja] = stabilny
    // [?] = niestabilny
    // [^] = overflow dodatni
    // [v] = overflow ujemny
    if(pos < len)
    {
        char stabilityChar = data[pos];
        if(stabilityChar == '?')
        {
            _stable = false;
            pos++;
        }
        else if(stabilityChar == '^')
        {
            _stable = false;  // Overflow to też niestabilność
            pos++;
        }
        else if(stabilityChar == 'v')
        {
            _stable = false;  // Overflow to też niestabilność
            pos++;
        }
        else if(stabilityChar == ' ')
        {
            _stable = true;
            pos++;
        }
        // Jeśli nie ma tego znaku, zakładamy stabilny
    }

    // KROK 4: Pomiń SPACJĘ po znaku stabilności
    if(pos < len && data[pos] == ' ')
        pos++;

    // KROK 5: Sprawdź ZNAK wartości (+/-)
    // [spacja] lub [+] = dodatnia
    // [-] = ujemna
    bool isNegative = false;
    if(pos < len)
    {
        char signChar = data[pos];
        if(signChar == '-')
        {
            isNegative = true;
            pos++;
        }
        else if(signChar == '+' || signChar == ' ')
        {
            isNegative = false;
            pos++;
        }
        // Jeśli nie ma znaku, zakładamy dodatnią
    }

    // KROK 6: Pomiń dodatkowe spacje przed MASĄ
    while(pos < len && (data[pos] == ' ' || data[pos] == '\t'))
        pos++;

    // KROK 7: Parsuj MASĘ (wartość numeryczną)
    QByteArray valueBytes;
    bool hasDigit = false;

    // Zbieraj cyfry, kropki, przecinki
    while(pos < len)
    {
        char ch = data[pos];

        if(ch >= '0' && ch <= '9')
        {
            valueBytes.append(ch);
            hasDigit = true;
            pos++;
        }
        else if(ch == '.' || ch == ',')
        {
            valueBytes.append('.');  // Normalizuj przecinek na kropkę
            pos++;
        }
        else
            break;  // Koniec wartości numerycznej
    }

    if(!hasDigit)
        return false;  // Brak wartości numerycznej

    // KROK 8: Sprawdź czy jest dodatkowa cyfra w nawiasach [n]
    if(pos < len && data[pos] == '[')
    {
        pos++;  // Pomiń '['

        // Pobierz cyfrę
        if(pos < len && data[pos] >= '0' && data[pos] <= '9')
        {
            valueBytes.append(data[pos]);
            pos++;
        }

        // Pomiń ']'
        if(pos < len && data[pos] == ']')
            pos++;
    }

    // Konwertuj wartość na double
    bool conversionOk;
    _value = valueBytes.toDouble(&conversionOk);

    if(!conversionOk)
        return false;

    // Zastosuj znak ujemny jeśli był wykryty
    if(isNegative)
        _value = -_value;

    // KROK 9: Pomiń SPACJĘ przed jednostką
    while(pos < len && (data[pos] == ' ' || data[pos] == '\t'))
        pos++;

    // KROK 10: Zbierz JEDNOSTKĘ (tylko litery, max 3 znaki)
    QByteArray unitBytes;
    while(pos < len && unitBytes.size() < 3)
    {
        char ch = data[pos];

        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        {
            unitBytes.append(ch);
            pos++;
        }
        else
            break;
    }

    _unitStr = QString::fromLatin1(unitBytes);
    _unit = parseUnitFromString(_unitStr);

    return true;
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
