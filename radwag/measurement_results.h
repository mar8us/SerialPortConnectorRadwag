#ifndef MEASUREMENT_RESULTS_H
#define MEASUREMENT_RESULTS_H

// #include "measurement.h"
#include <QString>
#include <qjsonobject.h>
class Measurement;

class MeasurementResults
{
public:
    MeasurementResults();
    MeasurementResults(const MeasurementResults& other);

    QString getMeasurementId() const;

    // Podstawowe wyniki
    double getApparentVolume(int precision = 4) const;     // Objętość pozorna
    double getApparentDensity(int precision = 4) const;    // Gęstość pozorna
    double getRelativeDensity(int precision = 2) const;    // Gęstość względna
    double getOpenPoresVolume(int precision = 4) const;    // Objętość porów otwartych
    double getTotalPorosity(int precision = 2) const;      // Porowatość całkowita

    // Wyniki dla pomiarów trzystopniowych
    double getOpenPorosity(int precision = 2) const;       // Porowatość otwarta
    double getClosedPorosity(int precision = 2) const;     // Porowatość zamknięta
    double getWaterAbsorption(int precision = 2) const;    // Nasiąkliwość wagowa

    double getTheoreticalDensity(int precision = 5) const;
    double getFluidDensity(int precision = 5) const;

    void setApparentVolume(double newApparentVolume);
    void setApparentDensity(double newApparentDensity);
    void setRelativeDensity(double newRelativeDensity);
    void setOpenPoresVolume(double newOpenPoresVolume);
    void setTotalPorosity(double newTotalPorosity);
    void setOpenPorosity(double newOpenPorosity);
    void setClosedPorosity(double newClosedPorosity);
    void setWaterAbsorption(double newWaterAbsorption);

    bool calculateResults(const Measurement* measurement);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString measurementId;

    double apparentVolume;       // Objętość pozorna
    double apparentDensity;      // Gęstość pozorna
    double relativeDensity;      // Gęstość względna
    double totalPorosity;        // Porowatość całkowita
    double openPoresVolume;      // Objętość porów otwartych
    double openPorosity;         // Porowatość otwarta (dla trzystopniowych)
    double closedPorosity;       // Porowatość zamknięta (dla trzystopniowych)
    double waterAbsorption;      // Nasiąkliwość wagowa (dla trzystopniowych)

    double materialTheoreticalDensity;
    double fluidDensity;

    double calculateApparentVolume(const Measurement* measurement);
    double calculateApparentDensity(const Measurement* measurement);
    double calculateRelativeDensity(double apparentDensity, double materialDensity);
    double calculateOpenPoresVolume(const Measurement* measurement);
    double calculateTotalPorosity(double apparentDensity, double materialDensity);
    double calculateOpenPorosity(const Measurement* measurement);
    double calculateClosedPorosity();
    double calculateWaterAbsorption(const Measurement* measurement);

    void setMeasurementId(const QString& newMeasurementId);
};

#endif // MEASUREMENT_RESULTS_H
