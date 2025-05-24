#ifndef MEASUREMENT_RESULTS_H
#define MEASUREMENT_RESULTS_H

#include "measurement.h"

class MeasurementResults
{
public:
    MeasurementResults();
    MeasurementResults(const QString& id, const QString& measurementId);

    QString getMeasurementId() const;
    void setMeasurementId(const QString& newMeasurementId);

    // Podstawowe wyniki
    double getApparentVolume() const;     // Objętość pozorna
    double getApparentDensity() const;    // Gęstość pozorna
    double getRelativeDensity() const;    // Gęstość względna
    double getOpenPoresVolume() const;    // Objętość porów otwartych
    double getTotalPorosity() const;      // Porowatość całkowita

    // Wyniki dla pomiarów trzystopniowych
    double getOpenPorosity() const;       // Porowatość otwarta
    double getClosedPorosity() const;       // Porowatość zamknięta
    double getWaterAbsorption() const;    // Nasiąkliwość wagowa

    void setApparentVolume(double newApparentVolume);
    void setApparentDensity(double newApparentDensity);
    void setRelativeDensity(double newRelativeDensity);
    void setOpenPoresVolume(double newOpenPoresVolume);
    void setTotalPorosity(double newTotalPorosity);
    void setOpenPorosity(double newOpenPorosity);
    void setClosedPorosity(double newClosedPorosity);
    void setWaterAbsorption(double newWaterAbsorption);

    bool calculateResults(std::shared_ptr<const Measurement> &measurement);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString measurementId;

    double apparentVolume;       // Objętość pozorna
    double apparentDensity;      // Gęstość pozorna
    double relativeDensity;      // Gęstość względna
    double totalPorosity;        // Porowatość całkowita
    double openPoresVolume;     // Objętość porów otwartych
    double openPorosity;         // Porowatość otwarta (dla trzystopniowych)
    double closedPorosity;       // Porowatość zamknięta (dla trzystopniowych)
    double waterAbsorption;      // Nasiąkliwość wagowa (dla trzystopniowych)

    double calculateApparentVolume(std::shared_ptr<const Measurement> &measurement);
    double calculateApparentDensity(std::shared_ptr<const Measurement> &measurement);
    double calculateRelativeDensity(double apparentDensity, double materialDensity);
    double calculateOpenPoresVolume(std::shared_ptr<const Measurement> &measurement);
    double calculateTotalPorosity(double apparentDensity, double materialDensity);
    double calculateOpenPorosity(std::shared_ptr<const Measurement> &measurement);
    double calculateClosedPorosity();
    double calculateWaterAbsorption(std::shared_ptr<const Measurement> &measurement);
};

#endif // MEASUREMENT_RESULTS_H
