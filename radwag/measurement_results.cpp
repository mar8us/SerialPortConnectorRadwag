#include "measurement_results.h"
#include "measurement.h"
#include <cmath>

MeasurementResults::MeasurementResults()
    : apparentVolume(0.0)
    , apparentDensity(0.0)
    , relativeDensity(0.0)
    , openPoresVolume(0.0)
    , openPorosity(0.0)
    , closedPorosity(0.0)
    , waterAbsorption(0.0)
    , materialTheoreticalDensity(0.0)
    , fluidDensity(0.0)
{

}

MeasurementResults::MeasurementResults(const MeasurementResults& other)
    : apparentVolume(other.apparentVolume)
    , apparentDensity(other.apparentDensity)
    , relativeDensity(other.relativeDensity)
    , totalPorosity(other.totalPorosity)
    , openPoresVolume(other.openPoresVolume)
    , openPorosity(other.openPorosity)
    , closedPorosity(other.closedPorosity)
    , waterAbsorption(other.waterAbsorption)
    , materialTheoreticalDensity(other.materialTheoreticalDensity)
    , fluidDensity(other.fluidDensity)
    , measurementId(other.measurementId)
{

}

QString MeasurementResults::getMeasurementId() const
{
    return measurementId;
}

#include <cmath>

double MeasurementResults::getApparentVolume(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(apparentVolume * factor) / factor;
}

double MeasurementResults::getApparentDensity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(apparentDensity * factor) / factor;
}

double MeasurementResults::getRelativeDensity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(relativeDensity * factor) / factor;
}

double MeasurementResults::getRelativeDensityFraction(int precision) const
{
    double fraction = relativeDensity / 100.0;
    double factor = std::pow(10.0, precision);
    return std::round(fraction * factor) / factor;
}

double MeasurementResults::getOpenPoresVolume(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(openPoresVolume * factor) / factor;
}

double MeasurementResults::getTotalPorosity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(totalPorosity * factor) / factor;
}

double MeasurementResults::getTotalPorosityFraction(int precision) const
{
    double fraction = totalPorosity / 100.0;
    double factor = std::pow(10.0, precision);
    return std::round(fraction * factor) / factor;
}

double MeasurementResults::getOpenPorosity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(openPorosity * factor) / factor;
}

double MeasurementResults::getOpenPorosityFraction(int precision) const
{
    double fraction = openPorosity / 100.0;
    double factor = std::pow(10.0, precision);
    return std::round(fraction * factor) / factor;
}

double MeasurementResults::getClosedPorosity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(closedPorosity * factor) / factor;
}

double MeasurementResults::getClosedPorosityFraction(int precision) const
{
    double fraction = closedPorosity / 100.0;
    double factor = std::pow(10.0, precision);
    return std::round(fraction * factor) / factor;
}

double MeasurementResults::getWaterAbsorption(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(waterAbsorption * factor) / factor;
}

double MeasurementResults::getWaterAbsorptionFraction(int precision) const
{
    double fraction = waterAbsorption / 100.0;
    double factor = std::pow(10.0, precision);
    return std::round(fraction * factor) / factor;
}

double MeasurementResults::getTheoreticalDensity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(materialTheoreticalDensity * factor) / factor;
}

double MeasurementResults::getFluidDensity(int precision) const
{
    double factor = std::pow(10.0, precision);
    return std::round(fluidDensity * factor) / factor;
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

void MeasurementResults::setOpenPoresVolume(double newOpenPoresVolume)
{
    openPoresVolume = newOpenPoresVolume;
}

void MeasurementResults::setTotalPorosity(double newTotalPorosity)
{
    totalPorosity = newTotalPorosity;
}

void MeasurementResults::setOpenPorosity(double newOpenPorosity)
{
    openPorosity = newOpenPorosity;
}

void MeasurementResults::setClosedPorosity(double newClosedPorosity)
{
    closedPorosity = newClosedPorosity;
}

void MeasurementResults::setWaterAbsorption(double newWaterAbsorption)
{
    waterAbsorption = newWaterAbsorption;
}

bool MeasurementResults::calculateResults(const Measurement* measurement)
{
    if(!measurement)
        return false;

    // Sprawdzenie, czy pomiar zawiera wszystkie wymagane dane
    if(!measurement->hasAllRequiredMeasurements())
        return false;

    materialTheoreticalDensity = measurement->getSampleMaterialDensity();
    fluidDensity = measurement->getFluidDensity();

    apparentVolume = calculateApparentVolume(measurement);
    if(apparentVolume <= 0.0)
        return false;

    // Obliczanie gęstości pozornej
    apparentDensity = calculateApparentDensity(measurement);
    if(apparentDensity <= 0.0)
        return false;

    // Obliczanie gęstości względnej
    relativeDensity = calculateRelativeDensity(apparentDensity, materialTheoreticalDensity);
    totalPorosity = calculateTotalPorosity(apparentDensity, materialTheoreticalDensity);

    // Dla pomiarów trzystopniowych oblicz porowatość otwartą i nasiąkliwość
    if(measurement->isThreeType())
    {
        openPoresVolume = calculateOpenPoresVolume(measurement);
        openPorosity = calculateOpenPorosity(measurement);
        closedPorosity = calculateClosedPorosity();
        waterAbsorption = calculateWaterAbsorption(measurement);
    }

    setMeasurementId(measurement->getId());
    return true;
}

double MeasurementResults::calculateApparentVolume(const Measurement* measurement)
{
    // Obliczenie objętości pozornej: V = (m_s - m_w) / ρ_cieczy
    double sampleDryMass = measurement->getSampleDryMass();
    double sampleInFluidMass = measurement->getSampleInFluidMass();

    if(fluidDensity <= 0.0)
        return 0.0;

    // Objętość pozorna w cm³
    return (sampleDryMass - sampleInFluidMass) / fluidDensity;
}

// double MeasurementResults::calculateApparentVolume(const Measurement* measurement)
// {
//     // Obliczenie objętości pozornej: V = (m_n - m_w) / ρ_cieczy
//     // Zgodnie z wzorem laboratoryjnym z dokumentacji
//     double saturatedMass = measurement->getSampleSaturatedMass();      // m_n
//     double sampleInFluidMass = measurement->getSampleInFluidMass();    // m_w
//     double fluidDensity = measurement->getFluidDensity();              // ρ_cieczy

//     if(fluidDensity <= 0.0 || saturatedMass <= sampleInFluidMass)
//         return 0.0;

//     // Objętość pozorna w cm³ (zgodnie z dokumentacją laboratoryjną)
//     return (saturatedMass - sampleInFluidMass) / fluidDensity;
// }

double MeasurementResults::calculateApparentDensity(const Measurement* measurement)
{
    // Obliczenie gęstości pozornej: ρ_p = m_s / V = m_s * ρ_cieczy / (m_s - m_w)
    double sampleDryMass = measurement->getSampleDryMass();
    double sampleInFluidMass = measurement->getSampleInFluidMass();

    if(sampleDryMass <= 0.0 || (sampleDryMass - sampleInFluidMass) <= 0.0)
        return 0.0;

    // Gęstość pozorna w g/cm³
    return (sampleDryMass * fluidDensity) / (sampleDryMass - sampleInFluidMass);
}

double MeasurementResults::calculateRelativeDensity(double apparentDensity, double materialDensity)
{
    // Obliczenie gęstości względnej (szczelności): s = (ρ_p / ρ_r) * 100%
    if(materialDensity <= 0.0)
        return 0.0;

    // Gęstość względna w %
    return (apparentDensity / materialDensity) * 100.0;
}

double MeasurementResults::calculateOpenPoresVolume(const Measurement* measurement)
{
    // Obliczenie objętości porów otwartych: V_porów = (m_n - m_s) / ρ_cieczy
    double sampleDryMass = measurement->getSampleDryMass();         // m_s
    double saturatedMass = measurement->getSampleSaturatedMass();   // m_n

    if(saturatedMass <= sampleDryMass || fluidDensity <= 0.0)
        return 0.0;

    // Objętość porów otwartych w cm³ (przy masach w g i gęstości w g/cm³)
    return (saturatedMass - sampleDryMass) / fluidDensity;
}

double MeasurementResults::calculateTotalPorosity(double apparentDensity, double materialDensity)
{
    if (materialDensity <= 0)
        return 0.0;
    return (1.0 - (apparentDensity / materialDensity)) * 100.0;
}

double MeasurementResults::calculateOpenPorosity(const Measurement* measurement)
{
    // Obliczenie porowatości otwartej: P_o = [(m_n - m_s) / (m_n - m_w)] * 100%
    double sampleDryMass = measurement->getSampleDryMass();             // m_s
    double sampleInFluidMass = measurement->getSampleInFluidMass();         // m_w
    double saturatedMass = measurement->getSampleSaturatedMass(); // m_n

    if(saturatedMass <= sampleDryMass || (saturatedMass - sampleInFluidMass) <= 0.0)
        return 0.0;

    // Porowatość otwarta w %
    return ((saturatedMass - sampleDryMass) / (saturatedMass - sampleInFluidMass)) * 100.0;
}

double MeasurementResults::calculateClosedPorosity()
{
    if(totalPorosity < 0.0 || openPorosity < 0.0)
        return 0.0;

    if(openPorosity > totalPorosity + 0.001)
        return 0.0;

    double result = totalPorosity - openPorosity;
    return std::max(0.0, result);
}

double MeasurementResults::calculateWaterAbsorption(const Measurement* measurement)
{
    // Obliczenie nasiąkliwości wagowej: N = [(m_n - m_s) / m_s] * 100%
    double sampleDryMass = measurement->getSampleDryMass();             // m_s
    double sampleSaturatedMass = measurement->getSampleSaturatedMass(); // m_n

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
    resultsObj["openPoresVolume"] = openPoresVolume;
    resultsObj["totalPorosity"] = totalPorosity;
    resultsObj["openPorosity"] = openPorosity;
    resultsObj["closedPorosity"] = closedPorosity;
    resultsObj["waterAbsorption"] = waterAbsorption;
    resultsObj["materialTheoreticalDensity"] = materialTheoreticalDensity;
    resultsObj["fluidDensity"] = fluidDensity;
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
    if(json.contains("openPoresVolume"))
        openPoresVolume = json["openPoresVolume"].toDouble();
    if(json.contains("totalPorosity"))
        totalPorosity = json["totalPorosity"].toDouble();
    if(json.contains("openPorosity"))
        openPorosity = json["openPorosity"].toDouble();
    if(json.contains("closedPorosity"))
        closedPorosity = json["closedPorosity"].toDouble();
    if(json.contains("waterAbsorption"))
        waterAbsorption = json["waterAbsorption"].toDouble();
    if(json.contains("materialTheoreticalDensity"))
        materialTheoreticalDensity = json["materialTheoreticalDensity"].toDouble();
    if(json.contains("fluidDensity"))
        fluidDensity = json["fluidDensity"].toDouble();
}
