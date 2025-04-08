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

    // Wyniki dla pomiarów trzystopniowych
    double getOpenPorosity() const;       // Porowatość otwarta
    double getWaterAbsorption() const;    // Nasiąkliwość wagowa

    void setApparentVolume(double newApparentVolume);
    void setApparentDensity(double newApparentDensity);
    void setRelativeDensity(double newRelativeDensity);
    void setOpenPorosity(double newOpenPorosity);
    void setWaterAbsorption(double newWaterAbsorption);

    bool calculateResults(std::shared_ptr<const Measurement> &measurement);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString measurementId;

    double apparentVolume;       // Objętość pozorna
    double apparentDensity;      // Gęstość pozorna
    double relativeDensity;      // Gęstość względna
    double openPorosity;         // Porowatość otwarta (dla trzystopniowych)
    double waterAbsorption;      // Nasiąkliwość wagowa (dla trzystopniowych)

    double calculateApparentVolume(std::shared_ptr<const Measurement> &measurement);
    double calculateApparentDensity(std::shared_ptr<const Measurement> &measurement);
    double calculateRelativeDensity(double apparentDensity, double materialDensity);
    double calculateOpenPorosity(std::shared_ptr<const Measurement> &measurement);
    double calculateWaterAbsorption(std::shared_ptr<const Measurement> &measurement);
};

#endif // MEASUREMENT_RESULTS_H
