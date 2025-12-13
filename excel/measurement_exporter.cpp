#include "measurement_exporter.h"

#include "../utils.h"
#include "../config.h"
#include "section_excel.h"
#include "style_formatter.h"
#include <QDebug>
#include <qelapsedtimer.h>

const QColor MeasurementExporter::LIGHT_BLUE(153, 204, 255);
const QColor MeasurementExporter::LIGHT_GRAY(230, 230, 230);
const QColor MeasurementExporter::LIGHT_GRAY_H(170, 170, 170);
const QColor MeasurementExporter::UNIT_FONT(64, 64, 64);

constexpr int START_ROW       = 1;
constexpr int START_COL       = 1;

constexpr int SPACING_SMALL   = 1;
constexpr int SPACING_MEDIUM  = 2;
constexpr int SPACING_LARGE   = 3;

constexpr int COL_COUNT  = 3;
constexpr int KEY_COL    = START_ROW - 1;
constexpr int VALUE_COL  = START_ROW;
constexpr int UNIT_COL   = START_ROW + 1;

MeasurementExporter::MeasurementExporter(const QString& fileName, const Measurement* measurement)
    : ExportExcelBase(fileName)
    , m_formatsInitialized(false)
    , m_analysisResult(nullptr)
{
    if(measurement)
        m_measurements.append(measurement);
    initializeFormats();
}

MeasurementExporter::MeasurementExporter(const QString& fileName, const QList<const Measurement*>& measurements)
    : ExportExcelBase(fileName)
    , m_measurements(measurements)
    , m_formatsInitialized(false)
    , m_analysisResult(nullptr)
{
    initializeFormats();
}

MeasurementExporter::MeasurementExporter(const QString &fileName, const AnalysisResult *analysisResult)
    : ExportExcelBase(fileName)
    , m_analysisResult(analysisResult)
    , m_measurements(analysisResult->getMeasurementsList())
    , m_formatsInitialized(false)
{
    initializeFormats();
}

bool MeasurementExporter::exportData()
{
    if(!validateData())
        return false;

    try
    {
        setupColumnSize(COL_COUNT, COL_WIDTH);

        int measureCount = 1;
        int currentRow = START_ROW;
        int maxWidth   = 0;

        for(const Measurement* measurement : m_measurements)
        {
            auto titleRange = addMeasurementTitle(measurement, measureCount, currentRow, START_COL);
            currentRow = titleRange.endRow + SPACING_SMALL;
            maxWidth = std::max(maxWidth, titleRange.colCount());

            auto generalRange = exportGeneralDataSection(measurement, currentRow, START_COL);
            currentRow = generalRange.endRow + SPACING_MEDIUM;
            maxWidth = std::max(maxWidth, generalRange.colCount());

            auto dataRange = exportMeasurementDataSection(measurement, currentRow, START_COL);
            currentRow = dataRange.endRow + SPACING_MEDIUM;
            maxWidth = std::max(maxWidth, dataRange.colCount());

            auto resultsRange = exportCalculationResultsSection(measurement, currentRow, START_COL);
            currentRow = resultsRange.endRow + SPACING_LARGE;
            maxWidth = std::max(maxWidth, resultsRange.colCount());

            measureCount++;
        }

        int tableStartCol = START_COL + maxWidth + SPACING_MEDIUM;

        if(m_analysisResult)
        {
#ifdef TABLE_FULL_STAT
            SectionRange fullTableRange = exportFullStatisticalTable(m_analysisResult, START_ROW, tableStartCol);
            currentRow = fullTableRange.endRow + SPACING_LARGE;
#else
            SectionRange analysisRange = exportAnalysisSection(m_analysisResult, currentRow, tableStartCol);
            currentRow = analysisRange.endRow + SPACING_LARGE;
#endif
        }
        else
        {
            auto tableRange = exportMeasurementTableSection(m_measurements, START_ROW, tableStartCol);
            currentRow = tableRange.endRow + SPACING_LARGE;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        qCritical() << "Error export:" << e.what();
        return false;
    }
    catch (...)
    {
        qCritical() << "Unknown error";
        return false;
    }
}

SectionRange MeasurementExporter::exportGeneralDataSection(const Measurement* measurement, int startRow, int startCol)
{
    Section generalSection("Dane ogólne", getSectionTitleFormat(), this, getCurrentRow(), startCol);

    int seriaRow = generalSection.addRow({"Seria:", measurement->getSampleName()});
    int materialRow = generalSection.addRow({"Materiał:", measurement->getSample()->getMaterial().getName()});
    int densityRow = generalSection.addRow({"Gęstość teoretyczna materiału:", measurement->getSampleMaterialDensity(), "g/cm³"});

    int liquidRow = generalSection.addRow({"Ciecz:", measurement->getFluidName()});
    int tempRow = generalSection.addRow({"Temperatura cieczy:", measurement->getFluidTemperature(), "°C"});
    int liquidDensityRow = generalSection.addRow({"Gęstość cieczy:", measurement->getFluidDensity(), "g/cm³"});

    setupStandardSectionFormat(generalSection);

    generalSection.setCellFormat(densityRow, VALUE_COL, getDecimalFormat(1));
    generalSection.setCellFormat(tempRow, VALUE_COL, getIntegerFormat());
    generalSection.setCellFormat(liquidDensityRow, VALUE_COL, getDecimalFormat(5));

    if(measurement->isThreeType())
    {
        int methodRow = generalSection.addRow({"Metoda nasycania:", utils::getSaturationMethodName(measurement->getSaturationMethod())});
        int startRow = generalSection.addRow({"Rozpoczęcie nasycania:", measurement->getSaturationBeginDate().toString("dd-MM-yyyy hh:mm:ss")});
        int timeRow = generalSection.addRow({"Czas nasycania:", measurement->getSaturationTime(), "min"});

        generalSection.setCellFormat(startRow, VALUE_COL, getDateTimeFormat());
        generalSection.setCellFormat(timeRow, VALUE_COL, getIntegerFormat());
    }

    int typeRow = generalSection.addRow({"Typ pomiaru:", measurement->isThreeType() ? "III st." : "II st."});
    int authorRow = generalSection.addRow({"Wykonawca:", measurement->getAuthor()});
    int dateRow = generalSection.addRow({"Data/godzina:", measurement->getDate().toString("dd-MM-yyyy hh:mm")});

    int commentRow = generalSection.addRow({"Komentarze:", measurement->getComments()});

    generalSection.setCellFormat(dateRow, VALUE_COL, getDateTimeFormat());

    generalSection.render();

    StyleFormatter fmt(getDocument());
    int endCol = generalSection.getStartCol() + generalSection.getTotalWidth() - 1;
    fmt.addBottomLine(generalSection.getStartRow(), generalSection.getStartCol(), endCol, QXlsx::Format::BorderThick, LIGHT_BLUE);

    return generalSection.getRange();
}

SectionRange MeasurementExporter::exportMeasurementDataSection(const Measurement* measurement, int startRow, int startCol)
{
    Section measurementDataSection("Dane pomiarowe", getSectionTitleFormat(), this, getCurrentRow(), startCol);

    int dryMassRow = measurementDataSection.addRow({"Masa suchej próbki (ms):", measurement->getSampleDryMass(), "g"});
    int fluidMassRow = measurementDataSection.addRow({"Masa próbki zanurzonej (mw):", measurement->getSampleInFluidMass(), "g"});

    setupStandardSectionFormat(measurementDataSection);

    measurementDataSection.setCellFormat(dryMassRow, VALUE_COL, getDecimalFormat(4));
    measurementDataSection.setCellFormat(fluidMassRow, VALUE_COL, getDecimalFormat(4));

    if(measurement->isThreeType())
    {
        int saturatedMassRow = measurementDataSection.addRow({"Masa próbki nasyconej (mn):", measurement->getSampleSaturatedMass(), "g"});
        measurementDataSection.setCellFormat(saturatedMassRow, VALUE_COL, getDecimalFormat(4));
    }

    measurementDataSection.render();

    StyleFormatter fmt(getDocument());
    int endCol = measurementDataSection.getStartCol() + measurementDataSection.getTotalWidth() - 1;
    fmt.addBottomLine(measurementDataSection.getStartRow(), measurementDataSection.getStartCol(), endCol, QXlsx::Format::BorderThick, LIGHT_BLUE);

    return measurementDataSection.getRange();
}

SectionRange MeasurementExporter::exportCalculationResultsSection(const Measurement* measurement, int startRow, int startCol)
{
    if(!hasValidResults(measurement))
        return SectionRange();

    const MeasurementResults& results = measurement->getResults();
    Section resultsSection("Obliczenia", getSectionTitleFormat(), this, getCurrentRow(), startCol);

    setupStandardSectionFormat(resultsSection);

    int apparentDensityRow = resultsSection.addRow({"Gęstość pozorna (ρp):", results.getApparentDensity(), "g/cm³"});
    int relativeDensityRow = resultsSection.addRow({"Gęstość względna:", results.getRelativeDensityFraction(), "%"});
    int apparentVolumeRow = resultsSection.addRow({"Objętość pozorna (Vp):", results.getApparentVolume(), "cm³"});

    resultsSection.setCellFormat(apparentDensityRow, VALUE_COL, getDecimalFormat(4));
    resultsSection.setCellFormat(relativeDensityRow, VALUE_COL, getPercentageFormat(2));
    resultsSection.setCellFormat(apparentVolumeRow, VALUE_COL, getDecimalFormat(4));

    if(measurement->isThreeType())
    {
        int openPoresRow = resultsSection.addRow({"Objętość porów otwartych:", results.getOpenPoresVolume(), "cm³"});
        int totalPorosityRow = resultsSection.addRow({"Porowatość całkowita:", results.getTotalPorosityFraction(), "%"});
        int openPorosityRow = resultsSection.addRow({"Porowatość otwarta:", results.getOpenPorosityFraction(), "%"});
        int closedPorosityRow = resultsSection.addRow({"Porowatość zamknięta:", results.getClosedPorosityFraction(), "%"});
        int absorptionRow = resultsSection.addRow({"Nasiąkliwość wagowa:", results.getWaterAbsorptionFraction(), "%"});

        resultsSection.setCellFormat(openPoresRow, VALUE_COL, getDecimalFormat(4));
        resultsSection.setCellFormat(totalPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(openPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(closedPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(absorptionRow, VALUE_COL, getPercentageFormat(2));
    }
    else
    {
        int totalPorosityRow = resultsSection.addRow({"Porowatość całkowita:", results.getTotalPorosityFraction(), "%"});
        resultsSection.setCellFormat(totalPorosityRow, VALUE_COL, getPercentageFormat(2));
    }

    resultsSection.render();

    StyleFormatter fmt(getDocument());
    int endCol = resultsSection.getStartCol() + resultsSection.getTotalWidth() - 1;
    fmt.addBottomLine(resultsSection.getStartRow(), resultsSection.getStartCol(), endCol, QXlsx::Format::BorderThick, LIGHT_BLUE);

    return resultsSection.getRange();
}

SectionRange MeasurementExporter::exportMeasurementTableSection(const QList<const Measurement*>& measurements, int startRow, int startCol)
{
    if(measurements.isEmpty())
        return SectionRange();

    Section tableSection("Tabela pomiarów i wyników", getTitleFormat(), this, startRow, startCol);

    QVariantList headerRow;
    headerRow << "Parametr";
    for(int i = 0; i < measurements.size(); i++)
        headerRow << QString("Pomiar %1 %2 %3").arg(i + 1).arg(" (" + measurements[i]->getSampleName() + ") ").arg(measurements[i]->getDate().toString("dd-MM-yyyy hh:mm"));
    int headerRowIndex = tableSection.addRow(headerRow);

    QVariantList dryMassRow;
    dryMassRow << "Masa sucha [g]";
    for(const Measurement* m : measurements)
        dryMassRow << m->getSampleDryMass();
    int dryMassRowIndex = tableSection.addRow(dryMassRow);

    QVariantList fluidMassRow;
    fluidMassRow << "Masa w cieczy [g]";
    for(const Measurement* m : measurements)
        fluidMassRow << m->getSampleInFluidMass();
    int fluidMassRowIndex = tableSection.addRow(fluidMassRow);

    QVariantList saturatedMassRow;
    saturatedMassRow << "Masa nasycona [g]";
    for(const Measurement* m : measurements)
    {
        if(m->isThreeType())
            saturatedMassRow << m->getSampleSaturatedMass();
        else
            saturatedMassRow << "-";
    }
    int saturatedMassRowIndex = tableSection.addRow(saturatedMassRow);

    QVariantList densityRow;
    densityRow << "Gęstość pozorna [g/cm³]";
    for(const Measurement* m : measurements)
        densityRow << m->getResults().getApparentDensity();
    int densityRowIndex = tableSection.addRow(densityRow);

    QVariantList porosityRow;
    porosityRow << "Porowatość całkowita [%]";
    for(const Measurement* m : measurements)
        porosityRow << m->getResults().getTotalPorosityFraction();
    int porosityRowIndex = tableSection.addRow(porosityRow);

    for(int col = 1; col <= measurements.size(); col++)
    {
        tableSection.setCellFormat(dryMassRowIndex, col, getDecimalFormat(4));
        tableSection.setCellFormat(fluidMassRowIndex, col, getDecimalFormat(4));
        tableSection.setCellFormat(saturatedMassRowIndex, col, getDecimalFormat(4));
        tableSection.setCellFormat(densityRowIndex, col, getDecimalFormat(4));
        tableSection.setCellFormat(porosityRowIndex, col, getPercentageFormat(2));
    }

    QXlsx::Format headerFormat = getTableHeaderFormat();
    headerFormat.setPatternBackgroundColor(LIGHT_GRAY_H);
    headerFormat.setFillPattern(QXlsx::Format::PatternSolid);
    tableSection.setRowFormat(headerRowIndex, headerFormat);

    QXlsx::Format labelFormat = getKeyFormat();
    labelFormat.setPatternBackgroundColor(LIGHT_GRAY);
    labelFormat.setFillPattern(QXlsx::Format::PatternSolid);
    tableSection.setColumnFormat(KEY_COL, labelFormat);

    tableSection.render();

    StyleFormatter formatTableSection(getDocument());

    formatTableSection.setGridBorder(tableSection.getStartRow(), tableSection.getStartCol(),
                      tableSection.getStartRow() + tableSection.getRowCount(), tableSection.getStartCol() + tableSection.getTotalWidth() - 1,
                      QXlsx::Format::BorderThin);



    return tableSection.getRange();//totalRange;
}

SectionRange MeasurementExporter::exportFullStatisticalTable(const AnalysisResult* analysisResult, int startRow, int startCol)
{
    if (!analysisResult || !analysisResult->isValid()) {
        return SectionRange{startRow, startCol, startRow, startCol};
    }

    Section tableSection("Analiza statystyczna", getTitleFormat(), this, startRow, startCol);
    tableSection.addRow({"Seria pomiarowa", analysisResult->getSeriesName()});
    tableSection.addRow({"Materiał", analysisResult->getMaterialName()});
    tableSection.addRow({"Liczba pomiarów", QString::number(analysisResult->getMeasuresCount())});
    tableSection.addRow({"Poziom ufności", QString("%1%").arg(analysisResult->getConfidenceLevel() * 100, 0, 'f', 0)});

    tableSection.render();
    auto headerRange = tableSection.getRange();
    int currentRow = headerRange.endRow + SPACING_MEDIUM;
    struct ResultRow {
        QString parameterName;
        const BaseAnalysisResult* result;
    };

    QList<ResultRow> resultRows;

    if(analysisResult->getDryMassResult().isValid())
        resultRows.append({"Masa sucha [g]", &analysisResult->getDryMassResult()});
    if(analysisResult->getWetMassResult().isValid())
        resultRows.append({"Masa w cieczy [g]", &analysisResult->getWetMassResult()});
    if(analysisResult->getSaturatedMassResult().isValid())
        resultRows.append({"Masa nasycona [g]", &analysisResult->getSaturatedMassResult()});
    if(analysisResult->getDensityResult().isValid())
        resultRows.append({"Gęstość pozorna [g/cm³]", &analysisResult->getDensityResult()});

    const auto &relativeDensityResult = analysisResult->getRelativeDensityResult();
    if(relativeDensityResult.isValid())
        resultRows.append({QString("Gęstość względna [%1]").arg(relativeDensityResult.getUnitSymbol()), &relativeDensityResult});

    if(analysisResult->getPorosityResult().isValid())
        resultRows.append({"Porowatość całkowita [%]", &analysisResult->getPorosityResult()});

    if(resultRows.isEmpty())
        return SectionRange{startRow, startCol, currentRow, startCol + 5};

    QVariantList headers;
    headers << "Parametr";

    const auto& measurements = analysisResult->getMeasurements();
    for(int i = 0; i < measurements.size(); ++i)
        headers << QString("Pomiar %1 (%2)").arg(i + 1).arg(measurements[i]->getEndDate().toString("dd-MM-yyyy hh:mm"));

    headers << "Średnia";
    headers << "Odch. std.";
    headers << "Współ. zmienności";
    headers << "Niepewność pomiarowa";
    headers << "Podsumowanie";

    Section statisticalTable("", QXlsx::Format(), this, currentRow, startCol);

    int headerRowIndex = statisticalTable.addRow(headers);

    QList<int> dataRowIndices;
    for (const auto& row : resultRows)
    {
        QVariantList rowData;
        rowData << row.parameterName;

        const auto& individualValues = row.result->getIndividualValues();
        for(double value : individualValues)
            rowData << value;

        rowData << row.result->getMean();
        rowData << row.result->getStandardDeviation();
        rowData << row.result->getVariationCoefficient() / 100.0;
        rowData << QString("±%1 %2").arg(QString::number(row.result->getUncertainty(), 'f', 4), row.result->getUnitSymbol());
        rowData << row.result->getFinalResult();

        int rowIndex = statisticalTable.addRow(rowData);

        QString name = row.result->getAnalysisTypeName();

        dataRowIndices.append(rowIndex);
    }

    QXlsx::Format headerFormat = getTableHeaderFormat();
    headerFormat.setPatternBackgroundColor(LIGHT_GRAY_H);
    headerFormat.setFillPattern(QXlsx::Format::PatternSolid);
    statisticalTable.setRowFormat(headerRowIndex, headerFormat);

    QXlsx::Format parameterFormat = getKeyFormat();
    parameterFormat.setFontBold(true);

    for(int i = 0; i < dataRowIndices.size(); ++i)
    {
        int rowIndex = dataRowIndices[i];
        const auto& result = resultRows[i].result;
        const auto& individualValues = result->getIndividualValues();
        QString name = result->getAnalysisTypeName();

        statisticalTable.setCellFormat(rowIndex, 0, parameterFormat);

        for(int col = 1; col <= individualValues.size(); ++col)
            if(result->getUnitSymbol() == "%")
                statisticalTable.setCellFormat(rowIndex, col, getPercentageFormat(2));
            else
                statisticalTable.setCellFormat(rowIndex, col, getDecimalFormat(4));

        int statStartCol = 1 + individualValues.size();
        statisticalTable.setCellFormat(rowIndex, statStartCol, getDecimalFormat(4));
        statisticalTable.setCellFormat(rowIndex, statStartCol + 1, getDecimalFormat(4));
        statisticalTable.setCellFormat(rowIndex, statStartCol + 2, getPercentageFormat(2));
    }

    statisticalTable.render();
    auto tableRange = statisticalTable.getRange();

    StyleFormatter formatter(getDocument());
    formatter.setGridBorder(tableRange.startRow, tableRange.startCol, tableRange.endRow, tableRange.endCol, QXlsx::Format::BorderThin);

    SectionRange totalRange;
    totalRange.startRow = headerRange.startRow;
    totalRange.startCol = std::min(headerRange.startCol, tableRange.startCol);
    totalRange.endRow = tableRange.endRow;
    totalRange.endCol = std::max(headerRange.endCol, tableRange.endCol);

    return totalRange;
}


SectionRange MeasurementExporter::exportAnalysisSection(const AnalysisResult* analysisResult, int startRow, int startCol)
{
    const DensityAnalysisResult& densityResult = analysisResult->getDensityResult();
    const PorosityAnalysisResult& porosityResult = analysisResult->getPorosityResult();

    SectionRange densityRange = exportDensityAnalysisSection(densityResult, startRow, startCol);
    SectionRange porosityRange = exportPorosityAnalysisSection(porosityResult, densityRange.endRow + SPACING_LARGE, startCol);

    SectionRange totalRange;
    totalRange.startRow = densityRange.startRow;
    totalRange.startCol = std::min(densityRange.startCol, porosityRange.startCol);
    totalRange.endRow = porosityRange.endRow;
    totalRange.endCol = std::max(densityRange.endCol, porosityRange.endCol);
    return totalRange;
}

SectionRange MeasurementExporter::exportDensityAnalysisSection(const DensityAnalysisResult& densityResult, int startRow, int startCol)
{
    Section densitySection(QString("Analiza gęstości pozornej (Seria: %1)").arg(m_analysisResult->getSeriesName()), getTitleFormat(), this, startRow, startCol);

    // Nagłówki kolumn dla gęstości
    QVariantList densityHeaderRow;
    densityHeaderRow << "Parametr" << "Wartość";
    int densityHeaderRowIndex = densitySection.addRow(densityHeaderRow);

    // Średnia gęstość
    QVariantList densityMeanRow;
    densityMeanRow << "Średnia gęstość [g/cm³]" << densityResult.getMean();
    int densityMeanRowIndex = densitySection.addRow(densityMeanRow);

    // Odchylenie standardowe gęstości
    QVariantList densityStdDevRow;
    densityStdDevRow << "Odchylenie standardowe [g/cm³]" << densityResult.getStandardDeviation();
    int densityStdDevRowIndex = densitySection.addRow(densityStdDevRow);

    // Współczynnik zmienności gęstości
    QVariantList densityCvRow;
    densityCvRow << "Współczynnik zmienności [%]" << densityResult.getVariationCoefficient() / 100;
    int densityCvRowIndex = densitySection.addRow(densityCvRow);

    // Niepewność pomiarowa gęstości
    QVariantList densityUncertaintyRow;
    densityUncertaintyRow << "Niepewność pomiarowa [g/cm³]" << densityResult.getUncertainty();
    int densityUncertaintyRowIndex = densitySection.addRow(densityUncertaintyRow);

    // Podsumowanie gęstości
    QVariantList densitySummaryRow;
    densitySummaryRow << "Podsumowanie [g/cm³]" << densityResult.getFinalResult();
    int densitySummaryRowIndex = densitySection.addRow(densitySummaryRow);

    // Formatowanie tabeli gęstości
    QXlsx::Format headerFormat = getTableHeaderFormat();
    headerFormat.setPatternBackgroundColor(LIGHT_GRAY_H);
    headerFormat.setFillPattern(QXlsx::Format::PatternSolid);
    densitySection.setRowFormat(densityHeaderRowIndex, headerFormat);

    QXlsx::Format labelFormat = getKeyFormat();
    labelFormat.setPatternBackgroundColor(LIGHT_GRAY);
    labelFormat.setFillPattern(QXlsx::Format::PatternSolid);
    densitySection.setColumnFormat(0, labelFormat);

    QXlsx::Format summaryFormat = getKeyFormat();
    summaryFormat.setFontBold(true);
    summaryFormat.setPatternBackgroundColor(QColor(220, 230, 241));
    summaryFormat.setFillPattern(QXlsx::Format::PatternSolid);
    densitySection.setRowFormat(densitySummaryRowIndex, summaryFormat);

    // Formatowanie liczb dla gęstości
    densitySection.setCellFormat(densityMeanRowIndex, 1, getDecimalFormat(3));
    densitySection.setCellFormat(densityStdDevRowIndex, 1, getDecimalFormat(4));
    densitySection.setCellFormat(densityCvRowIndex, 1, getPercentageFormat(2));
    densitySection.setCellFormat(densityUncertaintyRowIndex, 1, getDecimalFormat(3));

    densitySection.render();

    // Obramowanie tabeli gęstości
    StyleFormatter formatDensitySection(getDocument());
    formatDensitySection.setGridBorder(
        densitySection.getStartRow(),
        densitySection.getStartCol(),
        densitySection.getStartRow() + densitySection.getRowCount() - 1,
        densitySection.getStartCol() + densitySection.getTotalWidth() - 1,
        QXlsx::Format::BorderThin
        );

    return densitySection.getRange();
}

SectionRange MeasurementExporter::exportPorosityAnalysisSection(const PorosityAnalysisResult& porosityResult, int startRow, int startCol)
{
    Section porositySection(QString("Analiza porowatości całkowitej (Seria: %1)").arg(m_analysisResult->getSeriesName()), getTitleFormat(), this, startRow, startCol);

    // Nagłówki kolumn dla porowatości
    QVariantList porosityHeaderRow;
    porosityHeaderRow << "Parametr" << "Wartość";
    int porosityHeaderRowIndex = porositySection.addRow(porosityHeaderRow);

    // Średnia porowatość
    QVariantList porosityMeanRow;
    porosityMeanRow << "Średnia porowatość [%]" << porosityResult.getMean() / 100;
    int porosityMeanRowIndex = porositySection.addRow(porosityMeanRow);

    // Odchylenie standardowe porowatości
    QVariantList porosityStdDevRow;
    porosityStdDevRow << "Odchylenie standardowe [%]" << porosityResult.getStandardDeviation() / 100;
    int porosityStdDevRowIndex = porositySection.addRow(porosityStdDevRow);

    // Współczynnik zmienności porowatości
    QVariantList porosityCvRow;
    porosityCvRow << "Współczynnik zmienności [%]" << porosityResult.getVariationCoefficient() / 100;
    int porosityCvRowIndex = porositySection.addRow(porosityCvRow);

    // Niepewność pomiarowa porowatości
    QVariantList porosityUncertaintyRow;
    porosityUncertaintyRow << "Niepewność pomiarowa [%]" << porosityResult.getUncertainty() / 100;
    int porosityUncertaintyRowIndex = porositySection.addRow(porosityUncertaintyRow);

    // Podsumowanie porowatości
    QVariantList porositySummaryRow;
    porositySummaryRow << "Podsumowanie [%]" << porosityResult.getFinalResult();
    int porositySummaryRowIndex = porositySection.addRow(porositySummaryRow);

    // Formatowanie tabeli porowatości
    QXlsx::Format headerFormat = getTableHeaderFormat();
    headerFormat.setPatternBackgroundColor(LIGHT_GRAY_H);
    headerFormat.setFillPattern(QXlsx::Format::PatternSolid);
    porositySection.setRowFormat(porosityHeaderRowIndex, headerFormat);

    QXlsx::Format labelFormat = getKeyFormat();
    labelFormat.setPatternBackgroundColor(LIGHT_GRAY);
    labelFormat.setFillPattern(QXlsx::Format::PatternSolid);
    porositySection.setColumnFormat(0, labelFormat);

    QXlsx::Format summaryFormat = getKeyFormat();
    summaryFormat.setFontBold(true);
    summaryFormat.setPatternBackgroundColor(QColor(220, 230, 241));
    summaryFormat.setFillPattern(QXlsx::Format::PatternSolid);
    porositySection.setRowFormat(porositySummaryRowIndex, summaryFormat);

    // Formatowanie liczb dla porowatości (wszystkie jako procenty)
    porositySection.setCellFormat(porosityMeanRowIndex, 1, getPercentageFormat(2));
    porositySection.setCellFormat(porosityStdDevRowIndex, 1, getPercentageFormat(2));
    porositySection.setCellFormat(porosityCvRowIndex, 1, getPercentageFormat(2));
    porositySection.setCellFormat(porosityUncertaintyRowIndex, 1, getPercentageFormat(2));

    porositySection.render();

    // Obramowanie tabeli porowatości
    StyleFormatter formatPorositySection(getDocument());
    formatPorositySection.setGridBorder(
        porositySection.getStartRow(),
        porositySection.getStartCol(),
        porositySection.getStartRow() + porositySection.getRowCount() - 1,
        porositySection.getStartCol() + porositySection.getTotalWidth() - 1,
        QXlsx::Format::BorderThin
        );

    return porositySection.getRange();
}

void MeasurementExporter::setupColumnSize(int columns, int size)
{
    for(int col = 1; col <= columns; col++)
        setColumnWidth(col, size);
}

void MeasurementExporter::setupStandardSectionFormat(Section& section)
{
    section.setColumnFormat(KEY_COL, getKeyFormat());
    section.setColumnFormat(VALUE_COL, getValueFormat());
    section.setColumnFormat(UNIT_COL, getUnitFormat());
}

SectionRange MeasurementExporter::addMeasurementTitle(const Measurement* measurement, int index, int startRow, int startCol)
{
    QString measureTitle = QString("POMIAR %1 (%2): %3").arg(index).arg(measurement->getDate().toString("dd-MM-yyyy hh:mm")).arg(measurement->getSample()->getName());   //.arg(measurements[i]->getDate().toString("dd-MM-yyyy hh:mm")

    int width  = calculateRequiredMergeCells(measureTitle, getTitleFormat().font());
    int endRow = startRow;
    int endCol = startCol + width - 1;

    moveToPosition(startRow, startCol);
    mergeCells(startRow, startCol, endRow, endCol);
    writeValue(startRow, startCol, measureTitle, getTitleFormat());
    moveToNextRow();
    return SectionRange{startRow, startCol, endRow, endCol};
}

SectionRange MeasurementExporter::exportCommentsSection(const Measurement* measurement)
{
    return SectionRange();
}

bool MeasurementExporter::hasValidResults(const Measurement* measurement) const
{
    return measurement && measurement->hasResults();
}

bool MeasurementExporter::validateData() const
{
    if(m_measurements.isEmpty())
        return false;

    if(m_measurements.size() > 1)
    {
        int validCount = 0;
        for(const Measurement* measurement : m_measurements)
            if(measurement && hasValidResults(measurement))
                validCount++;

        if(validCount == 0)
            return false;
    }
    else
    {
        const Measurement* measurement = m_measurements.first();
        if(!measurement)
            return false;

        if(!hasValidResults(measurement))
            return false;
    }

    return true;
}

QXlsx::Format MeasurementExporter::createFormat(bool bold, int fontSize, QXlsx::Format::HorizontalAlignment hAlign, const QColor& backgroundColor, QXlsx::Format::BorderStyle borderStyle) const
{
    QXlsx::Format format;

    if(bold)
        format.setFontBold(true);
    if(fontSize > 0)
        format.setFontSize(fontSize);
    if(hAlign != QXlsx::Format::AlignHGeneral)
        format.setHorizontalAlignment(hAlign);
    if(backgroundColor.isValid())
        format.setPatternBackgroundColor(backgroundColor);
    if(borderStyle != QXlsx::Format::BorderNone)
        format.setBorderStyle(borderStyle);

    return format;
}


void MeasurementExporter::initializeFormats() const
{
    if(m_formatsInitialized)
        return;

    m_titleFormat = createFormat(true, 13, QXlsx::Format::AlignLeft, LIGHT_BLUE);
    m_sectionTitleFormat = createFormat(true, 11, QXlsx::Format::AlignLeft, LIGHT_GRAY_H);

    m_keyFormat = createFormat(true, 10, QXlsx::Format::AlignLeft);
    m_valueFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_unitFormat = createFormat(false, 9, QXlsx::Format::AlignLeft);
    m_unitFormat.setFontItalic(true);
    m_unitFormat.setFontColor(UNIT_FONT);

    m_decimalFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_decimalFormat.setNumberFormat("#,##0.0000");

    m_integerFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_integerFormat.setNumberFormat("#,##0");

    m_percentageFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_percentageFormat.setNumberFormat("0.00%");

    m_dateTimeFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_dateTimeFormat.setNumberFormat("dd-mm-yyyy hh:mm:ss");

    m_dateFormat = createFormat(false, 10, QXlsx::Format::AlignRight);
    m_dateFormat.setNumberFormat("dd-mm-yyyy");

    m_tableHeaderFormat = createFormat(true, 10, QXlsx::Format::AlignHCenter, LIGHT_GRAY, QXlsx::Format::BorderThin);

    m_formatsInitialized = true;
}

QXlsx::Format MeasurementExporter::getTitleFormat() const
{
    initializeFormats();
    return m_titleFormat;
}

QXlsx::Format MeasurementExporter::getSectionTitleFormat() const
{
    initializeFormats();
    return m_sectionTitleFormat;
}


QXlsx::Format MeasurementExporter::getKeyFormat() const
{
    initializeFormats();
    return m_keyFormat;
}

QXlsx::Format MeasurementExporter::getValueFormat() const
{
    initializeFormats();
    return m_valueFormat;
}

QXlsx::Format MeasurementExporter::getDecimalFormat(int precision) const
{
    initializeFormats();
    QXlsx::Format format = m_decimalFormat;

    QString numberFormat;
    if(precision == 0)
        numberFormat = "#,##0";
    else
    {
        QString decimals = QString("0").repeated(precision);
        numberFormat = QString("#,##0.%1").arg(decimals);
    }

    format.setNumberFormat(numberFormat);
    return format;
}

QXlsx::Format MeasurementExporter::getIntegerFormat() const
{
    initializeFormats();
    return m_integerFormat;
}

QXlsx::Format MeasurementExporter::getPercentageFormat(int precision) const
{
    initializeFormats();
    QXlsx::Format format = m_percentageFormat;

    QString decimals = QString("0").repeated(precision);
    QString numberFormat = QString("0.%1%").arg(decimals);

    format.setNumberFormat(numberFormat);
    return format;
}

QXlsx::Format MeasurementExporter::getDateTimeFormat() const
{
    initializeFormats();
    return m_dateTimeFormat;
}

QXlsx::Format MeasurementExporter::getDateFormat() const
{
    initializeFormats();
    return m_dateFormat;
}

QXlsx::Format MeasurementExporter::getUnitFormat() const
{
    initializeFormats();
    return m_unitFormat;
}

QXlsx::Format MeasurementExporter::getTableHeaderFormat() const
{
    initializeFormats();
    return m_tableHeaderFormat;
}



