#include "measure_statistic_analyzer.h"

#include <cmath>
#include <algorithm>
#include <map>
#include <QUuid>
#include <QJsonArray>
#include <QJsonDocument>
#include <qfiledevice.h>
#include "../../../../utils.h"

// =============================================================================
// TStudentTable
// =============================================================================

const std::map<std::pair<int, int>, double> TStudentTable::tTable =
{
    // (degrees_of_freedom, confidence_level*100) -> t_value

    // 90% confidence level (α = 0.10)
    {{1, 90}, 6.314}, {{2, 90}, 2.920}, {{3, 90}, 2.353}, {{4, 90}, 2.132},
    {{5, 90}, 2.015}, {{6, 90}, 1.943}, {{7, 90}, 1.895}, {{8, 90}, 1.860},
    {{9, 90}, 1.833}, {{10, 90}, 1.812}, {{11, 90}, 1.796}, {{12, 90}, 1.782},
    {{13, 90}, 1.771}, {{14, 90}, 1.761}, {{15, 90}, 1.753}, {{16, 90}, 1.746},
    {{17, 90}, 1.740}, {{18, 90}, 1.734}, {{19, 90}, 1.729}, {{20, 90}, 1.725},

    // 95% confidence level
    {{1, 95}, 12.706}, {{2, 95}, 4.303}, {{3, 95}, 3.182}, {{4, 95}, 2.776},
    {{5, 95}, 2.571}, {{6, 95}, 2.447}, {{7, 95}, 2.365}, {{8, 95}, 2.306},
    {{9, 95}, 2.262}, {{10, 95}, 2.228}, {{11, 95}, 2.201}, {{12, 95}, 2.179},
    {{13, 95}, 2.160}, {{14, 95}, 2.145}, {{15, 95}, 2.131}, {{16, 95}, 2.120},
    {{17, 95}, 2.110}, {{18, 95}, 2.101}, {{19, 95}, 2.093}, {{20, 95}, 2.086},

    // 99% confidence level
    {{1, 99}, 63.657}, {{2, 99}, 9.925}, {{3, 99}, 5.841}, {{4, 99}, 4.604},
    {{5, 99}, 4.032}, {{6, 99}, 3.707}, {{7, 99}, 3.499}, {{8, 99}, 3.355},
    {{9, 99}, 3.250}, {{10, 99}, 3.169}, {{11, 99}, 3.106}, {{12, 99}, 3.055},
    {{13, 99}, 3.012}, {{14, 99}, 2.977}, {{15, 99}, 2.947}, {{16, 99}, 2.921},
    {{17, 99}, 2.898}, {{18, 99}, 2.878}, {{19, 99}, 2.861}, {{20, 99}, 2.845}
};

double TStudentTable::getValue(int degreesOfFreedom, double confidenceLevel)
{
    int confidence = static_cast<int>(confidenceLevel * 100);

    auto it = tTable.find(std::make_pair(degreesOfFreedom, confidence));
    return it != tTable.end() ? it->second : 0.0;
}

// =============================================================================
// BaseAnalysisResult
// =============================================================================

BaseAnalysisResult::BaseAnalysisResult()
    : confidenceLevel(0.0)
    , analysisType(AnalysisType::Density)
    , theoreticalDensity(0.0)
    , mean(0.0)
    , standardDeviation(0.0)
    , standardError(0.0)
    , uncertainty(0.0)
    , variationCoefficient(0.0)
    , valid(false)
{
    analysisDate = QDateTime::currentDateTime();
    generateAnalysisId();
}

BaseAnalysisResult::BaseAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements,
                                       AnalysisType analysisType,
                                       double mean,
                                       double stdDev,
                                       double uncertainty,
                                       double confidenceLevel)
    : mean(mean)
    , standardDeviation(stdDev)
    , uncertainty(uncertainty)
    , confidenceLevel(confidenceLevel)
    , analysisType(analysisType)
    , measurements(measurements)
{
    analysisDate = QDateTime::currentDateTime();
    generateAnalysisId();

    if(!measurements.empty())
    {
        auto &measure = measurements[0];
        seriesName = measure->getSample()->getName();
        materialName = measure->getSample()->getMaterialName();
        theoreticalDensity = measure->getSampleMaterialDensity();

        for(const auto& measure : measurements)
        {
            measurementIds.push_back(measure->getId());
            measurementDates.push_back(measure->getDate());
            authors.push_back(measure->getAuthor());
        }
    }

    calculateStatistics();
    validateResults();
}

void BaseAnalysisResult::generateAnalysisId()
{
    QString typePrefix = (analysisType == AnalysisType::Density) ? "DENS" : "POR";
    analysisId = typePrefix + "_" + QUuid::createUuid().toString().remove('{').remove('}');
}

void BaseAnalysisResult::calculateStatistics()
{
    int measuresCount = getMeasuresCount();
    if(measuresCount <= 1)
        return;

    standardError = standardDeviation / std::sqrt(measuresCount);
    variationCoefficient = (standardDeviation / mean) * 100.0;
}

void BaseAnalysisResult::validateResults()
{
    valid = true;
    warnings.clear();
    validationErrors.clear();
    int measuresCount = getMeasuresCount();

    if(measuresCount < 2)
    {
        validationErrors.append("Liczba próbek musi być >= 2");
        valid = false;
    }

    if(mean <= 0)
    {
        validationErrors.append("Średnia wartość musi być > 0");
        valid = false;
    }

    if(standardDeviation < 0)
    {
        validationErrors.append("Odchylenie standardowe nie może być ujemne");
        valid = false;
    }

    if(variationCoefficient > 5.0)
        warnings.append(QString("Duży współczynnik zmienności: %1%").arg(variationCoefficient, 0, 'f', 2));

    if(analysisType == AnalysisType::Density)
        double relativeDensity = mean / theoreticalDensity;
    else if(analysisType == AnalysisType::Porosity)
    {
        if(mean < 0 || mean > 100)
        {
            validationErrors.append("Porowatość musi być w zakresie 0-100%");
            valid = false;
        }
    }
}

QString BaseAnalysisResult::getAnalysisId() const
{
    return analysisId;
}

QDateTime BaseAnalysisResult::getAnalysisDate() const
{
    return analysisDate;
}

int BaseAnalysisResult::getMeasuresCount() const
{
    return measurements.size();
}

double BaseAnalysisResult::getConfidenceLevel() const
{
    return confidenceLevel;
}

AnalysisType BaseAnalysisResult::getAnalysisType() const
{
    return analysisType;
}

QString BaseAnalysisResult::getMaterialName() const
{
    return materialName;
}

double BaseAnalysisResult::getTheoreticalDensity() const
{
    return theoreticalDensity;
}

QString BaseAnalysisResult::getSeriesName() const
{
    return seriesName;
}

double BaseAnalysisResult::getMean() const
{
    return mean;
}

double BaseAnalysisResult::getStandardDeviation() const
{
    return standardDeviation;
}

double BaseAnalysisResult::getStandardError() const
{
    return standardError;
}

double BaseAnalysisResult::getUncertainty() const
{
    return uncertainty;
}

double BaseAnalysisResult::getVariationCoefficient() const
{
    return variationCoefficient;
}

std::vector<double> BaseAnalysisResult::getIndividualValues() const
{
    return individualValues;
}

std::vector<QString> BaseAnalysisResult::getAuthors() const
{
    return authors;
}

std::vector<QDateTime> BaseAnalysisResult::getMeasurementDates() const
{
    return measurementDates;
}

bool BaseAnalysisResult::isValid() const
{
    return valid;
}

QStringList BaseAnalysisResult::getWarnings() const
{
    return warnings;
}

QStringList BaseAnalysisResult::getValidationErrors() const
{
    return validationErrors;
}

const std::vector<std::shared_ptr<const Measurement>>& BaseAnalysisResult::getMeasurements() const
{
    return measurements;
}

std::vector<QString> BaseAnalysisResult::getMeasurementsIDs() const
{
    return measurementIds;
}

SaturationMethod BaseAnalysisResult::getSaturationMethod() const
{
    return std::all_of(measurements.begin() + 1, measurements.end(),
                       [&](const auto& m) { return m->getSaturationMethod() == measurements[0]->getSaturationMethod(); })
               ? measurements[0]->getSaturationMethod() : SaturationMethod::None;
}


// =============================================================================
// DensityAnalysisResult
// =============================================================================

DensityAnalysisResult::DensityAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Density;
}

DensityAnalysisResult::DensityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements,
                                             double mean,
                                             double stdDev,
                                             double uncertainty,
                                             double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Density, mean, stdDev, uncertainty, confidenceLevel)
{
    for(const auto& measure : measurements)
        if(measure->hasResults())
            individualValues.push_back(measure->getResults().getApparentDensity());
}

QString DensityAnalysisResult::getFinalResult() const
{
    return QString("%1 ± %2 g/cm³").arg(mean, 0, 'f', 2).arg(uncertainty, 0, 'f', 3);
}

QString DensityAnalysisResult::getUnitSymbol() const
{
    return "g/cm³";
}

QString DensityAnalysisResult::getAnalysisTypeName() const
{
    return "Gęstość pozorna";
}

// =============================================================================
// PorosityAnalysisResult Implementation
// =============================================================================

PorosityAnalysisResult::PorosityAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Porosity;
}

PorosityAnalysisResult::PorosityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements,
                                               double mean,
                                               double stdDev,
                                               double uncertainty,
                                               double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Porosity, mean, stdDev, uncertainty, confidenceLevel)
{
    for(const auto& measure : measurements)
        if(measure->hasResults())
            individualValues.push_back(measure->getResults().getTotalPorosity());
}

QString PorosityAnalysisResult::getFinalResult() const
{
    return QString("%1 ± %2 %").arg(mean, 0, 'f', 1).arg(uncertainty, 0, 'f', 1);
}

QString PorosityAnalysisResult::getUnitSymbol() const
{
    return "%";
}

QString PorosityAnalysisResult::getAnalysisTypeName() const
{
    return "Porowatość całkowita";
}


// =============================================================================
// AnalysisResult.cpp
// =============================================================================

AnalysisResult::AnalysisResult()
    : confidenceLevel(0.0)
{
    analysisDate = QDateTime::currentDateTime();
    generateAnalysisId();
}

AnalysisResult::AnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : measurements(measurements)
    , confidenceLevel(confidenceLevel)
{
    analysisDate = QDateTime::currentDateTime();
    generateAnalysisId();
    calculateCompleteAnalysis();
}

void AnalysisResult::generateAnalysisId()
{
    analysisId = "COMPLETE_" + QUuid::createUuid().toString().remove('{').remove('}');
}

const std::vector<std::shared_ptr<const Measurement>>& AnalysisResult::getMeasurements() const
{
    return measurements;
}

void AnalysisResult::setMeasurements(const std::vector<std::shared_ptr<const Measurement>>& newMeasurements)
{
    measurements = newMeasurements;

    densityResult = DensityAnalysisResult();
    porosityResult = PorosityAnalysisResult();
}

const DensityAnalysisResult& AnalysisResult::getDensityResult() const
{
    return densityResult;
}

const PorosityAnalysisResult& AnalysisResult::getPorosityResult() const
{
    return porosityResult;
}

bool AnalysisResult::hasDensityResult() const
{
    return densityResult.isValid();
}

bool AnalysisResult::hasPorosityResult() const
{
    return porosityResult.isValid();
}

double AnalysisResult::getConfidenceLevel() const
{
    return confidenceLevel;
}

void AnalysisResult::setConfidenceLevel(double newConfidenceLevel)
{
    if(std::abs(confidenceLevel - newConfidenceLevel) > 0.001)
    {
        confidenceLevel = newConfidenceLevel;
        calculateCompleteAnalysis();
    }
}

bool AnalysisResult::calculateDensityAnalysis()
{
    if(!canCalculateDenistyAnalysis())
        return false;

    std::vector<double> densityValues = extractDensityValues();

    double mean = StatisticalAnalyzer::calculateMean(densityValues);
    double stdDev = StatisticalAnalyzer::calculateStandardDeviation(densityValues, mean);
    double stdError = StatisticalAnalyzer::calculateStandardError(stdDev, static_cast<int>(densityValues.size()));
    int degreesOfFreedom = static_cast<int>(densityValues.size()) - 1;
    double uncertainty = StatisticalAnalyzer::calculateUncertainty(stdError, degreesOfFreedom, confidenceLevel);

    densityResult = DensityAnalysisResult(measurements, mean, stdDev, uncertainty, confidenceLevel);

    return densityResult.isValid();
}

bool AnalysisResult::calculatePorosityAnalysis()
{
    if(!canCalculatePorosityAnalysis())
        return false;

    std::vector<double> porosityValues = extractPorosityValues();

    double mean = StatisticalAnalyzer::calculateMean(porosityValues);
    double stdDev = StatisticalAnalyzer::calculateStandardDeviation(porosityValues, mean);
    double stdError = StatisticalAnalyzer::calculateStandardError(stdDev, static_cast<int>(porosityValues.size()));
    int degreesOfFreedom = static_cast<int>(porosityValues.size()) - 1;
    double uncertainty = StatisticalAnalyzer::calculateUncertainty(stdError, degreesOfFreedom, confidenceLevel);

    porosityResult = PorosityAnalysisResult(measurements, mean, stdDev, uncertainty, confidenceLevel);

    return porosityResult.isValid();
}

bool AnalysisResult::calculateCompleteAnalysis()
{
    if(!calculateDensityAnalysis())
        return false;

    if(!calculatePorosityAnalysis())
        return false;

    return densityResult.isValid() && porosityResult.isValid();
}

QString AnalysisResult::getAnalysisId() const
{
    return analysisId;
}

QDateTime AnalysisResult::getAnalysisDate() const
{
    return analysisDate;
}

QString AnalysisResult::getMaterialName() const
{
    if(!measurements.empty())
        return measurements[0]->getSample()->getMaterialName();
    return QString();
}

QString AnalysisResult::getSeriesName() const
{
    if(!measurements.empty())
        return measurements[0]->getSample()->getName();
        return measurements[0]->getSample()->getName();
    return QString();
}

int AnalysisResult::getMeasuresCount() const
{
    return static_cast<int>(measurements.size());
}

bool AnalysisResult::isValid() const
{
    return canCalculate() && densityResult.isValid() && porosityResult.isValid();
}

bool AnalysisResult::isComplete() const
{
    return hasDensityResult() && hasPorosityResult();
}

bool AnalysisResult::canCalculate() const
{
    return canCalculateDenistyAnalysis() && canCalculatePorosityAnalysis();
}

bool AnalysisResult::canCalculateDenistyAnalysis() const
{
    return StatisticalAnalyzer::validateGroupCriteria(measurements, AnalysisType::Density);
}

bool AnalysisResult::canCalculatePorosityAnalysis() const
{
    return StatisticalAnalyzer::validateGroupCriteria(measurements, AnalysisType::Porosity);
}

QStringList AnalysisResult::getWarnings() const
{
    QStringList warnings;

    if(!canCalculate())
    {
        warnings.append("Nieprawidłowe pomiary - nie można obliczyć analiz");
        return warnings;
    }

    if(hasDensityResult())
    {
        QStringList densityWarnings = densityResult.getWarnings();
        for(const QString& warning : densityWarnings)
            warnings.append("Gęstość: " + warning);
    }

    if(hasPorosityResult())
    {
        QStringList porosityWarnings = porosityResult.getWarnings();
        for(const QString& warning : porosityWarnings)
            warnings.append("Porowatość: " + warning);
    }

    return warnings;
}

QStringList AnalysisResult::getValidationErrors() const
{
    QStringList errors;

    if(!densityResult.isValid())
    {
        QStringList densityErrors = densityResult.getValidationErrors();
        for(const QString& error : densityErrors)
            errors.append("Gęstość: " + error);
    }

    if(!porosityResult.isValid())
    {
        QStringList porosityErrors = porosityResult.getValidationErrors();
        for(const QString& error : porosityErrors)
            errors.append("Porowatość: " + error);
    }

    return errors;
}

QString AnalysisResult::getSummary() const
{
    QString summary;

    if(hasDensityResult())
        summary += densityResult.getFinalResult();

    if(hasPorosityResult())
    {
        if(!summary.isEmpty())
            summary += " | ";
        summary += porosityResult.getFinalResult();
    }

    if(summary.isEmpty())
        summary = "Brak wyników analizy";

    return summary;
}

QJsonObject AnalysisResult::toJson() const
{
    QJsonObject json;

    QJsonArray measurementsArray;
    for(const auto& measurement : measurements)
        measurementsArray.append(measurement->toJson());

    json["measurements"] = measurementsArray;
    json["analysisId"] = analysisId;
    json["analysisDate"] = analysisDate.toString(Qt::ISODate);
    json["confidenceLevel"] = confidenceLevel;

    return json;
}

void AnalysisResult::fromJson(const QJsonObject& json)
{
    // measurements.clear();
    // QJsonArray measurementsArray = json["measurements"].toArray();
    // for(const auto& value : measurementsArray)
    // {
    //     auto measurement = std::make_shared<Measurement>();
    //     measurement->fromJson(value.toObject());
    //     measurements.push_back(measurement);
    // }
    // analysisId = json["analysisId"].toString();
    // analysisDate = QDateTime::fromString(json["analysisDate"].toString(), Qt::ISODate);
    // confidenceLevel = json["confidenceLevel"].toDouble();

    // calculateCompleteAnalysis();
}


bool AnalysisResult::operator==(const AnalysisResult& other) const
{
    return analysisId == other.analysisId;
}

bool AnalysisResult::operator!=(const AnalysisResult& other) const
{
    return !(*this == other);
}

std::vector<double> AnalysisResult::extractDensityValues()
{
    std::vector<double> values;
    values.reserve(measurements.size());

    for(const auto& measurement : measurements)
        if(measurement->hasResults())
            values.push_back(measurement->getResults().getApparentDensity());

    return values;
}

std::vector<double> AnalysisResult::extractPorosityValues()
{
    std::vector<double> values;
    values.reserve(measurements.size());

    for(const auto& measurement : measurements)
        if(measurement->hasResults())
            values.push_back(measurement->getResults().getTotalPorosity());

    return values;
}


// =============================================================================
// StatisticalAnalyzer
// =============================================================================

bool StatisticalAnalyzer::validateGroupCriteria(const std::vector<std::shared_ptr<const Measurement>>& measurements, AnalysisType type)
{
    if(measurements.size() < 2)
        return false;

    const auto& reference = measurements[0];

    for(const auto& measure : measurements)
    {
        if(!areEqualSeries(reference.get(), measure.get()))
            return false;

        if(!validateMeasurementData(measure.get(), type))
            return false;

        // if(!areEqualMethods(reference, measure))
        //     return false;

        // if(!areEqualTheoreticalDensities(reference.get(), measure.get()))
        //     return false;
    }

    return true;
}

bool StatisticalAnalyzer::validateMeasurementData(const Measurement* measurement, AnalysisType type)
{
    return measurement->hasResults() && measurement->isCompleted() && (type == AnalysisType::Density ? measurement->getResults().getApparentDensity() : measurement->getResults().getTotalPorosity()) > 0.0;
}

bool StatisticalAnalyzer::areEqualSeries(const Measurement* m1, const Measurement* m2)
{
    return m1->getSample()->getName() == m2->getSample()->getName() && utils::compareDouble(m1->getSampleMaterialDensity(), m2->getSampleMaterialDensity());
}

bool StatisticalAnalyzer::areEqualMethods(const Measurement& m1, const Measurement& m2)
{
    return m1.getSaturationMethod() == m2.getSaturationMethod();
}


bool StatisticalAnalyzer::hasOutliers(const std::vector<double>& values, double threshold)
{
    if(values.size() < 3)
        return false;

    double mean = calculateMean(values);
    double stdDev = calculateStandardDeviation(values, mean);

    for(double value : values)
        if(std::abs(value - mean) > threshold * stdDev)
            return true;

    return false;
}

std::vector<int> StatisticalAnalyzer::findOutliers(const std::vector<double>& values, double threshold)
{
    std::vector<int> outliers;
    if (values.size() < 3)
        return outliers;

    double mean = calculateMean(values);
    double stdDev = calculateStandardDeviation(values, mean);

    for(size_t i = 0; i < values.size(); i++)
        if (std::abs(values[i] - mean) > threshold * stdDev)
            outliers.push_back(static_cast<int>(i));

    return outliers;
}

double StatisticalAnalyzer::calculateMean(const std::vector<double>& values)
{
    if(values.empty())
        return 0.0;

    double sum = 0.0;
    for(double value : values)
        sum += value;

    return sum / values.size();
}

double StatisticalAnalyzer::calculateStandardDeviation(const std::vector<double>& values, double mean)
{
    if(values.size() < 2)
        return 0.0;

    double sumSquaredDiff = 0.0;
    for(double value : values)
    {
        double diff = value - mean;
        sumSquaredDiff += diff * diff;
    }

    return std::sqrt(sumSquaredDiff / (values.size() - 1));
}

double StatisticalAnalyzer::calculateStandardError(double stdDev, int measuresCount)
{
    if(measuresCount < 1)
        return 0.0;
    return stdDev / std::sqrt(measuresCount);
}

double StatisticalAnalyzer::calculateUncertainty(double standardError, int degreesOfFreedom, double confidenceLevel)
{
    double tValue = TStudentTable::getValue(degreesOfFreedom, confidenceLevel);
    return tValue * standardError;
}

double StatisticalAnalyzer::calculateVariationCoefficient(double stdDev, double mean)
{
    if(mean == 0.0)
        return 0.0;
    return (stdDev / mean) * 100.0;
}
