#include "measurement_results.h"
#include <cmath>

MeasurementResults::MeasurementResults()
    : apparentVolume(0.0)
    , apparentDensity(0.0)
    , relativeDensity(0.0)
    , openPorosity(0.0)
    , waterAbsorption(0.0)
{

}

MeasurementResults::MeasurementResults(const QString& id, const QString& measurementId)
    : measurementId(measurementId)
    , apparentVolume(0.0)
    , apparentDensity(0.0)
    , relativeDensity(0.0)
    , openPorosity(0.0)
    , waterAbsorption(0.0)
{

}

QString MeasurementResults::getMeasurementId() const
{
    return measurementId;
}

double MeasurementResults::getApparentVolume() const
{
    return apparentVolume;
}

double MeasurementResults::getApparentDensity() const
{
    return apparentDensity;
}

double MeasurementResults::getRelativeDensity() const
{
    return relativeDensity;
}

double MeasurementResults::getOpenPorosity() const
{
    return openPorosity;
}

double MeasurementResults::getWaterAbsorption() const
{
    return waterAbsorption;
}

void MeasurementResults::setMeasurementId(const QString& newMeasurementId)
{
    measurementId = newMeasurementId;
}

void MeasurementResults::setApparentVolume(double newApparentVolume)
{
    apparentVolume = newApparentVolume;
}

void MeasurementResults::setApparentDensity(double newApparentDensity)
{
    apparentDensity = newApparentDensity;
}

void MeasurementResults::setRelativeDensity(double newRelativeDensity)
{
    relativeDensity = newRelativeDensity;
}

void MeasurementResults::setOpenPorosity(double newOpenPorosity)
{
    openPorosity = newOpenPorosity;
}

void MeasurementResults::setWaterAbsorption(double newWaterAbsorption)
{
    waterAbsorption = newWaterAbsorption;
}

bool MeasurementResults::calculateResults(const Measurement& measurement, double materialDensity)
{
    // Sprawdzenie, czy pomiar zawiera wszystkie wymagane dane
    if(!measurement.hasAllRequiredMeasurements())
        return false;

    // Obliczanie objętości pozornej
    apparentVolume = calculateApparentVolume(measurement);
    if(apparentVolume <= 0.0)
        return false;

    // Obliczanie gęstości pozornej
    apparentDensity = calculateApparentDensity(measurement);
    if(apparentDensity <= 0.0)
        return false;

    // Obliczanie gęstości względnej
    relativeDensity = calculateRelativeDensity(apparentDensity, materialDensity);

    // Dla pomiarów trzystopniowych oblicz porowatość otwartą i nasiąkliwość
    if(measurement.isThreeType())
    {
        openPorosity = calculateOpenPorosity(measurement);
        waterAbsorption = calculateWaterAbsorption(measurement);
    }

    return true;
}

double MeasurementResults::calculateApparentVolume(const Measurement& measurement)
{
    // Obliczenie objętości pozornej: V = (m_s - m_w) / ρ_cieczy
    double sampleDryMass = measurement.getSampleDryMass();
    double sampleInFluidMass = measurement.getSampleInFluidMass();
    double fluidDensity = measurement.getFluidDensity();

    if(fluidDensity <= 0.0)
        return 0.0;

    // Objętość pozorna w cm³
    return (sampleDryMass - sampleInFluidMass) / fluidDensity;
}

double MeasurementResults::calculateApparentDensity(const Measurement& measurement)
{
    // Obliczenie gęstości pozornej: ρ_p = m_s / V = m_s * ρ_cieczy / (m_s - m_w)
    double sampleDryMass = measurement.getSampleDryMass();
    double sampleInFluidMass = measurement.getSampleInFluidMass();
    double fluidDensity = measurement.getFluidDensity();

    if(sampleDryMass <= 0.0 || (sampleDryMass - sampleInFluidMass) <= 0.0)
        return 0.0;

    // Gęstość pozorna w g/cm³
    return (sampleDryMass * sampleInFluidMass) / (sampleDryMass - sampleInFluidMass);
}

double MeasurementResults::calculateRelativeDensity(double apparentDensity, double materialDensity)
{
    // Obliczenie gęstości względnej (szczelności): s = (ρ_p / ρ_r) * 100%
    if(materialDensity <= 0.0)
        return 0.0;

    // Gęstość względna w %
    return (apparentDensity / materialDensity) * 100.0;
}

double MeasurementResults::calculateOpenPorosity(const Measurement& measurement)
{
    // Obliczenie porowatości otwartej: P_o = [(m_n - m_s) / (m_n - m_w)] * 100%
    double sampleDryMass = measurement.getSampleDryMass();             // m_s
    double sampleInFluidMass = measurement.getSampleInFluidMass();         // m_w
    double saturatedMass = measurement.getSampleSaturatedMass(); // m_n

    if(saturatedMass <= sampleDryMass || (saturatedMass - sampleInFluidMass) <= 0.0)
        return 0.0;

    // Porowatość otwarta w %
    return ((saturatedMass - sampleDryMass) / (saturatedMass - sampleInFluidMass)) * 100.0;
}

double MeasurementResults::calculateWaterAbsorption(const Measurement& measurement)
{
    // Obliczenie nasiąkliwości wagowej: N = [(m_n - m_s) / m_s] * 100%
    double sampleDryMass = measurement.getSampleDryMass();             // m_s
    double sampleSaturatedMass = measurement.getSampleSaturatedMass(); // m_n

    if(sampleDryMass <= 0.0)
        return 0.0;

    // Nasiąkliwość wagowa w %
    return ((sampleSaturatedMass - sampleDryMass) / sampleDryMass) * 100.0;
}

QJsonObject MeasurementResults::toJson() const
{
    QJsonObject resultsObj;
    resultsObj["measurementId"] = measurementId;
    resultsObj["apparentVolume"] = apparentVolume;
    resultsObj["apparentDensity"] = apparentDensity;
    resultsObj["relativeDensity"] = relativeDensity;
    resultsObj["openPorosity"] = openPorosity;
    resultsObj["waterAbsorption"] = waterAbsorption;
    return resultsObj;
}

void MeasurementResults::fromJson(const QJsonObject &json)
{
    if(json.contains("measurementId"))
        measurementId = json["measurementId"].toString();
    if(json.contains("apparentVolume"))
        apparentVolume = json["apparentVolume"].toDouble();
    if(json.contains("apparentDensity"))
        apparentDensity = json["apparentDensity"].toDouble();
    if(json.contains("relativeDensity"))
        relativeDensity = json["relativeDensity"].toDouble();
    if(json.contains("openPorosity"))
        openPorosity = json["openPorosity"].toDouble();
    if(json.contains("waterAbsorption"))
        waterAbsorption = json["waterAbsorption"].toDouble();
}
