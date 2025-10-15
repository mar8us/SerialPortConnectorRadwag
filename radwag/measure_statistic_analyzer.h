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
    None,
    Density,
    Porosity,
    Mass
};

class BaseAnalysisResult
{
public:
    BaseAnalysisResult();
    BaseAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, AnalysisType analysisType, double confidenceLevel);

    virtual ~BaseAnalysisResult() = default;

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
    double getVariationCoefficientFraction() const;

    const std::vector<std::shared_ptr<const Measurement>>& getMeasurements() const;
    std::vector<QString> getMeasurementsIDs() const;
    std::vector<double> getIndividualValues() const;
    std::vector<QString> getAuthors() const;
    std::vector<QDateTime> getMeasurementDates() const;

    bool isValid() const;
    QStringList getWarnings() const;
    QStringList getValidationErrors() const;


    bool calculate();

    virtual QString getFinalResult() const = 0;
    virtual QString getUnitSymbol() const = 0;
    virtual QString getAnalysisTypeName() const = 0;
    virtual void extractValues() = 0;
    virtual bool canCalculate() const = 0;

protected:
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

    void validateResults();

    std::vector<std::shared_ptr<const Measurement>> measurements;
};

class DensityAnalysisResult : public BaseAnalysisResult
{
public:
    DensityAnalysisResult();
    DensityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
    virtual void extractValues() override;
    bool canCalculate() const override;
};

class PorosityAnalysisResult : public BaseAnalysisResult
{
public:
    PorosityAnalysisResult();
    PorosityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
    virtual void extractValues() override;
    bool canCalculate() const override;
};


// =============================================================================
// DryMassAnalysisResult
// =============================================================================

class DryMassAnalysisResult : public BaseAnalysisResult
{
public:
    DryMassAnalysisResult();
    DryMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
    virtual void extractValues() override;
    bool canCalculate() const override;
};


// =============================================================================
// WetMassAnalysisResult
// =============================================================================

class WetMassAnalysisResult : public BaseAnalysisResult
{
public:
    WetMassAnalysisResult();
    WetMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
    virtual void extractValues() override;
    bool canCalculate() const override;
};


// =============================================================================
// SaturatedMassAnalysisResult
// =============================================================================

class SaturatedMassAnalysisResult : public BaseAnalysisResult
{
public:
    SaturatedMassAnalysisResult();
    SaturatedMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel);

    QString getFinalResult() const override;
    QString getUnitSymbol() const override;
    QString getAnalysisTypeName() const override;
    virtual void extractValues() override;
    bool canCalculate() const override;
};


class AnalysisResult
{
public:
    AnalysisResult();
    AnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel = 0.95);

    const std::vector<std::shared_ptr<const Measurement>>& getMeasurements() const;
    const QList<const Measurement *> getMeasurementsList() const;
    void setMeasurements(const std::vector<std::shared_ptr<const Measurement>>& measurements);

    const DensityAnalysisResult& getDensityResult() const;
    const PorosityAnalysisResult& getPorosityResult() const;
    const DryMassAnalysisResult& getDryMassResult() const;
    const WetMassAnalysisResult& getWetMassResult() const;
    const SaturatedMassAnalysisResult& getSaturatedMassResult() const;

    double getConfidenceLevel() const;
    void setConfidenceLevel(double confidenceLevel);

    bool calculateDensityAnalysis();
    bool calculatePorosityAnalysis();
    bool calculateMassAnalyses();
    bool calculateCompleteAnalysis();

    QDateTime getAnalysisDate() const;
    QString getMaterialName() const;
    QString getSeriesName() const;
    int getMeasuresCount() const;

    bool isValid() const;
    bool isComplete() const;

    bool canCalculate() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);

private:
    std::vector<std::shared_ptr<const Measurement>> measurements;
    double confidenceLevel;

    DensityAnalysisResult densityResult;
    PorosityAnalysisResult porosityResult;
    DryMassAnalysisResult dryMassResult;
    WetMassAnalysisResult wetMassResult;
    SaturatedMassAnalysisResult saturatedMassResult;

    QDateTime analysisDate;
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
