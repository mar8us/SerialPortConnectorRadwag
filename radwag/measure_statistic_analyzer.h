#ifndef MEASURE_STATISTIC_ANALYZER_H
#define MEASURE_STATISTIC_ANALYZER_H

#include <vector>
#include <QString>
#include <QDateTime>
#include <QStringList>
#include "measurement.h"


class TStudentTable
{
public:
    static double getValue(int degreesOfFreedom, double confidenceLevel);

private:
    static const std::map<std::pair<int, int>, double> tTable;
};


enum class AnalysisType
{
    Density,
    Porosity
};

class BaseAnalysisResult
{
public:
    BaseAnalysisResult();
    BaseAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, AnalysisType analysisType, double mean, double stdDev, double uncertainty, double confidenceLevel);

    virtual ~BaseAnalysisResult() = default;

    QString getAnalysisId() const;
    QDateTime getAnalysisDate() const;
    double getConfidenceLevel() const;
    AnalysisType getAnalysisType() const;

    int getMeasuresCount() const;
    QString getMaterialName() const;
    SaturationMethod getSaturationMethod() const;
    double getTheoreticalDensity() const;
    QString getSeriesName() const;

    double getMean() const;                    // Średnia
    double getStandardDeviation() const;       // Odchylenie standardowe
    double getStandardError() const;           // Błąd standardowy
    double getUncertainty() const;             // Niepewność
    double getVariationCoefficient() const;    // Współczynnik zmienności [%]

    const std::vector<std::shared_ptr<const Measurement>>& getMeasurements() const;
    std::vector<QString> getMeasurementsIDs() const;
    std::vector<double> getIndividualValues() const;
    std::vector<QString> getAuthors() const;
    std::vector<QDateTime> getMeasurementDates() const;

    bool isValid() const;
    QStringList getWarnings() const;
    QStringList getValidationErrors() const;

    virtual QString getFinalResult() const = 0;
    virtual QString getUnitSymbol() const = 0;
    virtual QString getAnalysisTypeName() const = 0;

protected:
    QString analysisId;
    QDateTime analysisDate;
    double confidenceLevel;
    AnalysisType analysisType;

    QString materialName;
    double theoreticalDensity;
    QString seriesName;
    std::vector<QString> measurementIds;
    std::vector<QString> authors;
    std::vector<QDateTime> measurementDates;

    double mean;
    double standardDeviation;
    double standardError;
    double uncertainty;
    double variationCoefficient;
    std::vector<double> individualValues;

    bool valid;
    QStringList warnings;
    QStringList validationErrors;

    void generateAnalysisId();
    void calculateStatistics();
    void validateResults();

    std::vector<std::shared_ptr<const Measurement>> measurements;
};

class DensityAnalysisResult : public BaseAnalysisResult
{
public:
    DensityAnalysisResult();
    DensityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements,
                          double mean,
                          double stdDev,
                          double uncertainty,
                          double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
};

class PorosityAnalysisResult : public BaseAnalysisResult
{
public:
    PorosityAnalysisResult();
    PorosityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements,
                           double mean,
                           double stdDev,
                           double uncertainty,
                           double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
};


class AnalysisResult
{
public:
    AnalysisResult();
    AnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel = 0.95);

    const std::vector<std::shared_ptr<const Measurement>>& getMeasurements() const;
    void setMeasurements(const std::vector<std::shared_ptr<const Measurement>>& measurements);

    const DensityAnalysisResult& getDensityResult() const;
    const PorosityAnalysisResult& getPorosityResult() const;

    bool hasDensityResult() const;
    bool hasPorosityResult() const;

    double getConfidenceLevel() const;
    void setConfidenceLevel(double confidenceLevel);

    bool calculateDensityAnalysis();
    bool calculatePorosityAnalysis();
    bool calculateCompleteAnalysis();

    QString getAnalysisId() const;
    QDateTime getAnalysisDate() const;
    QString getMaterialName() const;
    QString getSeriesName() const;
    int getMeasuresCount() const;
    QString getSummary() const;

    bool isValid() const;
    bool isComplete() const;

    bool canCalculate() const;
    bool canCalculateDenistyAnalysis() const;
    bool canCalculatePorosityAnalysis() const;

    QStringList getWarnings() const;
    QStringList getValidationErrors() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);

    bool operator==(const AnalysisResult& other) const;
    bool operator!=(const AnalysisResult& other) const;

private:
    std::vector<double> extractDensityValues();
    std::vector<double> extractPorosityValues();

    std::vector<std::shared_ptr<const Measurement>> measurements;
    double confidenceLevel;

    DensityAnalysisResult densityResult;
    PorosityAnalysisResult porosityResult;

    QString analysisId;
    QDateTime analysisDate;

    void generateAnalysisId();
    bool validateMeasurements();
};


class StatisticalAnalyzer
{
public:
    static bool validateGroupCriteria(const std::vector<std::shared_ptr<const Measurement>>& measurements, AnalysisType type);
    static bool validateMeasurementData(const Measurement* measurement, AnalysisType type);
    static bool areEqualSeries(const Measurement* m1, const Measurement* m2);
    static bool areEqualMethods(const Measurement& m1, const Measurement& m2);

    static bool hasOutliers(const std::vector<double>& values, double threshold = 2.0);
    static std::vector<int> findOutliers(const std::vector<double>& values, double threshold = 2.0);

    static double calculateMean(const std::vector<double>& values);
    static double calculateStandardDeviation(const std::vector<double>& values, double mean);
    static double calculateStandardError(double stdDev, int measuresCount);
    static double calculateUncertainty(double standardError, int degreesOfFreedom, double confidenceLevel);
    static double calculateVariationCoefficient(double stdDev, double mean);
};

#endif
