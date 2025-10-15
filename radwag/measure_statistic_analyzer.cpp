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
    , analysisType(AnalysisType::None)
    , theoreticalDensity(0.0)
    , mean(0.0)
    , standardDeviation(0.0)
    , standardError(0.0)
    , uncertainty(0.0)
    , variationCoefficient(0.0)
    , valid(false)
{

}

BaseAnalysisResult::BaseAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, AnalysisType analysisType, double confidenceLevel)
                                        : measurements(measurements)
                                        , analysisType(analysisType)
                                        , confidenceLevel(confidenceLevel)
                                        , theoreticalDensity(0.0)
                                        , mean(0.0)
                                        , standardDeviation(0.0)
                                        , standardError(0.0)
                                        , uncertainty(0.0)
                                        , variationCoefficient(0.0)
                                        , valid(false)
{
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

double BaseAnalysisResult::getVariationCoefficientFraction() const
{
    return round(variationCoefficient / 100.0 * 10000.0) / 10000.0;
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

bool BaseAnalysisResult::calculate()
{
    if(!canCalculate())
        return false;

    extractValues();

    mean = StatisticalAnalyzer::calculateMean(individualValues);
    standardDeviation = StatisticalAnalyzer::calculateStandardDeviation(individualValues, mean);
    double stdError = StatisticalAnalyzer::calculateStandardError(standardDeviation, static_cast<int>(individualValues.size()));
    int degreesOfFreedom = static_cast<int>(individualValues.size()) - 1;
    uncertainty = StatisticalAnalyzer::calculateUncertainty(stdError, degreesOfFreedom, confidenceLevel);

    standardError = standardDeviation / std::sqrt(getMeasuresCount());
    variationCoefficient = (standardDeviation / mean) * 100.0;

    validateResults();
    return isValid();
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

DensityAnalysisResult::DensityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Density, confidenceLevel)
{

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

void DensityAnalysisResult::extractValues()
{
    for(const auto& measure : measurements)
        if(measure->hasResults())
            individualValues.push_back(measure->getResults().getApparentDensity());
}

bool DensityAnalysisResult::canCalculate() const
{
    return StatisticalAnalyzer::validateGroupCriteria(measurements, AnalysisType::Density);
}

// =============================================================================
// PorosityAnalysisResult Implementation
// =============================================================================

PorosityAnalysisResult::PorosityAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Porosity;
}

PorosityAnalysisResult::PorosityAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Porosity, confidenceLevel)
{

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

void PorosityAnalysisResult::extractValues()
{
    for(const auto& measure : measurements)
        if(measure->hasResults())
            individualValues.push_back(measure->getResults().getTotalPorosity());
}

bool PorosityAnalysisResult::canCalculate() const
{
    return StatisticalAnalyzer::validateGroupCriteria(measurements, AnalysisType::Porosity);
}

// =============================================================================
// DryMassAnalysisResult
// =============================================================================

DryMassAnalysisResult::DryMassAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Mass;
}

DryMassAnalysisResult::DryMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Mass, confidenceLevel)
{

}



QString DryMassAnalysisResult::getFinalResult() const
{
    return QString("%1 ± %2 g").arg(mean, 0, 'f', 4).arg(uncertainty, 0, 'f', 4);
}

QString DryMassAnalysisResult::getUnitSymbol() const
{
    return "g";
}

QString DryMassAnalysisResult::getAnalysisTypeName() const
{
    return "Masa sucha (ms)";
}

void DryMassAnalysisResult::extractValues()
{
    for(const auto& measure : measurements)
        individualValues.push_back(measure->getSampleDryMass());
}

bool DryMassAnalysisResult::canCalculate() const
{
    return measurements.size() >= 2;
}

// =============================================================================
// WetMassAnalysisResult
// =============================================================================

WetMassAnalysisResult::WetMassAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Mass;
}

WetMassAnalysisResult::WetMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Mass, confidenceLevel)
{

}

QString WetMassAnalysisResult::getFinalResult() const
{
    return QString("%1 ± %2 g").arg(mean, 0, 'f', 4).arg(uncertainty, 0, 'f', 4);
}

QString WetMassAnalysisResult::getUnitSymbol() const
{
    return "g";
}

QString WetMassAnalysisResult::getAnalysisTypeName() const
{
    return "Masa w cieczy (mw)";
}

void WetMassAnalysisResult::extractValues()
{
    for(const auto& measure : measurements)
        individualValues.push_back(measure->getSampleInFluidMass());
}

bool WetMassAnalysisResult::canCalculate() const
{
    return measurements.size() >= 2;
}

// =============================================================================
// SaturatedMassAnalysisResult
// =============================================================================

SaturatedMassAnalysisResult::SaturatedMassAnalysisResult()
    : BaseAnalysisResult()
{
    analysisType = AnalysisType::Mass;
}

SaturatedMassAnalysisResult::SaturatedMassAnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : BaseAnalysisResult(measurements, AnalysisType::Mass, confidenceLevel)
{

}

QString SaturatedMassAnalysisResult::getFinalResult() const
{
    return QString("%1 ± %2 g").arg(mean, 0, 'f', 4).arg(uncertainty, 0, 'f', 4);
}

QString SaturatedMassAnalysisResult::getUnitSymbol() const
{
    return "g";
}

QString SaturatedMassAnalysisResult::getAnalysisTypeName() const
{
    return "Masa nasycona (mn)";
}

void SaturatedMassAnalysisResult::extractValues()
{
    for(const auto& measure : measurements)
        individualValues.push_back(measure->getSampleSaturatedMass());
}

bool SaturatedMassAnalysisResult::canCalculate() const
{
    return measurements.size() >= 2;
}


// =============================================================================
// AnalysisResult.cpp
// =============================================================================

AnalysisResult::AnalysisResult()
    : confidenceLevel(0.0)
{
    analysisDate = QDateTime::currentDateTime();
}

AnalysisResult::AnalysisResult(const std::vector<std::shared_ptr<const Measurement>>& measurements, double confidenceLevel)
    : measurements(measurements)
    , confidenceLevel(confidenceLevel)
{
    analysisDate = QDateTime::currentDateTime();
    calculateCompleteAnalysis();
}

const std::vector<std::shared_ptr<const Measurement>>& AnalysisResult::getMeasurements() const
{
    return measurements;
}

const QList<const Measurement*> AnalysisResult::getMeasurementsList() const
{
    QList<const Measurement*> list;
    for(const auto& measure : measurements)
        list.append(measure.get());
    return list;
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

const DryMassAnalysisResult& AnalysisResult::getDryMassResult() const
{
    return dryMassResult;
}

const WetMassAnalysisResult& AnalysisResult::getWetMassResult() const
{
    return wetMassResult;
}

const SaturatedMassAnalysisResult& AnalysisResult::getSaturatedMassResult() const
{
    return saturatedMassResult;
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
    densityResult = DensityAnalysisResult(measurements, confidenceLevel);
    return densityResult.calculate();
}

bool AnalysisResult::calculatePorosityAnalysis()
{
    porosityResult = PorosityAnalysisResult(measurements, confidenceLevel);
    return porosityResult.calculate();
}

bool AnalysisResult::calculateMassAnalyses()
{
    dryMassResult = DryMassAnalysisResult(measurements, confidenceLevel);
    wetMassResult = WetMassAnalysisResult(measurements, confidenceLevel);
    saturatedMassResult = SaturatedMassAnalysisResult(measurements, confidenceLevel);

    return dryMassResult.calculate() && wetMassResult.calculate() && saturatedMassResult.calculate();
}

bool AnalysisResult::calculateCompleteAnalysis()
{
    bool massSuccess = calculateMassAnalyses();
    bool densitySuccess = calculateDensityAnalysis();
    bool porositySuccess = calculatePorosityAnalysis();

    return massSuccess && densitySuccess && porositySuccess;
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
    return densityResult.isValid() && porosityResult.isValid() && dryMassResult.isValid() && wetMassResult.isValid() && saturatedMassResult.isValid();
}

bool AnalysisResult::canCalculate() const
{
    return densityResult.canCalculate() && porosityResult.canCalculate() && dryMassResult.canCalculate() && wetMassResult.canCalculate() && saturatedMassResult.canCalculate();
}

QJsonObject AnalysisResult::toJson() const
{
    QJsonObject json;

    // QJsonArray measurementsArray;
    // for(const auto& measurement : measurements)
    //     measurementsArray.append(measurement->toJson());

    // json["measurements"] = measurementsArray;
    // json["analysisId"] = analysisId;
    // json["analysisDate"] = analysisDate.toString(Qt::ISODate);
    // json["confidenceLevel"] = confidenceLevel;

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
