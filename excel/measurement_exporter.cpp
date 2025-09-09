#include "measurement_exporter.h"

#include "../utils.h"
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
{
    if(measurement)
        m_measurements.append(measurement);
    initializeFormats();
}

MeasurementExporter::MeasurementExporter(const QString& fileName, const QList<const Measurement*>& measurements)
    : ExportExcelBase(fileName)
    , m_measurements(measurements)
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

        if(m_measurements.size() > 1)
        {
            int tableStartCol = START_COL + maxWidth + SPACING_MEDIUM;
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
    int relativeDensityRow = resultsSection.addRow({"Gęstość względna:", results.getRelativeDensity()  / 100, "%"});
    int apparentVolumeRow = resultsSection.addRow({"Objętość pozorna (Vp):", results.getApparentVolume(), "cm³"});

    resultsSection.setCellFormat(apparentDensityRow, VALUE_COL, getDecimalFormat(4));
    resultsSection.setCellFormat(relativeDensityRow, VALUE_COL, getPercentageFormat(2));
    resultsSection.setCellFormat(apparentVolumeRow, VALUE_COL, getDecimalFormat(4));

    if(measurement->isThreeType())
    {
        int openPoresRow = resultsSection.addRow({"Objętość porów otwartych:", results.getOpenPoresVolume(), "cm³"});
        int totalPorosityRow = resultsSection.addRow({"Porowatość całkowita:", results.getTotalPorosity() / 100, "%"});
        int openPorosityRow = resultsSection.addRow({"Porowatość otwarta:", results.getOpenPorosity() / 100, "%"});
        int closedPorosityRow = resultsSection.addRow({"Porowatość zamknięta:", results.getClosedPorosity() / 100, "%"});
        int absorptionRow = resultsSection.addRow({"Nasiąkliwość wagowa:", results.getWaterAbsorption() / 100, "%"});

        resultsSection.setCellFormat(openPoresRow, VALUE_COL, getDecimalFormat(4));
        resultsSection.setCellFormat(totalPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(openPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(closedPorosityRow, VALUE_COL, getPercentageFormat(2));
        resultsSection.setCellFormat(absorptionRow, VALUE_COL, getPercentageFormat(2));
    }
    else
    {
        int totalPorosityRow = resultsSection.addRow({"Porowatość całkowita:", results.getTotalPorosity() / 100, "%"});
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

    Section tableSection("Porównanie pomiarów", getTitleFormat(), this, startRow, startCol);

    QVariantList headerRow;
    headerRow << "Oznaczana wielkość";
    for(int i = 0; i < measurements.size(); i++)
        headerRow << QString("Pomiar %1").arg(i + 1);
    int headerRowIndex = tableSection.addRow(headerRow);

    QVariantList dryMassRow;
    dryMassRow << "ms [g]";
    for(const Measurement* m : measurements)
        dryMassRow << m->getSampleDryMass();
    int dryMassRowIndex = tableSection.addRow(dryMassRow);

    QVariantList fluidMassRow;
    fluidMassRow << "mw [g]";
    for(const Measurement* m : measurements)
        fluidMassRow << m->getSampleInFluidMass();
    int fluidMassRowIndex = tableSection.addRow(fluidMassRow);

    QVariantList saturatedMassRow;
    saturatedMassRow << "mn [g]";
    for(const Measurement* m : measurements)
    {
        if(m->isThreeType())
            saturatedMassRow << m->getSampleSaturatedMass();
        else
            saturatedMassRow << "-";
    }
    int saturatedMassRowIndex = tableSection.addRow(saturatedMassRow);

    QVariantList densityRow;
    densityRow << "dp [g/cm³]";
    for(const Measurement* m : measurements)
        densityRow << m->getResults().getApparentDensity();
    int densityRowIndex = tableSection.addRow(densityRow);

    QVariantList porosityRow;
    porosityRow << "Pc [%]";
    for(const Measurement* m : measurements)
        porosityRow << m->getResults().getTotalPorosity() / 100;
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

    QXlsx::Format legendTitleFormat = getTitleFormat();
    legendTitleFormat.setFontSize(10);

    int legendStartRow = startRow + tableSection.getRange().endRow + SPACING_MEDIUM;
    Section legendSection("Legenda oznaczeń", legendTitleFormat, this, legendStartRow, startCol);

    legendSection.addRow({"ms", "masa suchej próbki"});
    legendSection.addRow({"mw", "masa próbki zanurzonej w cieczy"});
    legendSection.addRow({"mn", "masa próbki nasyconej cieczą"});
    legendSection.addRow({"dp", "gęstość pozorna próbki"});
    legendSection.addRow({"Pc", "porowatość całkowita"});

    QXlsx::Format symbolFormat = getTableHeaderFormat();
    symbolFormat.setFontBold(true);
    symbolFormat.setFontItalic(true);

    QXlsx::Format descriptionFormat = getKeyFormat();
    descriptionFormat.setFontBold(false);
    descriptionFormat.setPatternBackgroundColor(LIGHT_GRAY);

    for(int i = 0; i < legendSection.getRowCount(); i++)
    {
        legendSection.setCellFormat(i, 0, symbolFormat);
        legendSection.setCellFormat(i, 1, descriptionFormat);
    }

    legendSection.render();

    SectionRange tableRange = tableSection.getRange();
    SectionRange legendRange = legendSection.getRange();

    SectionRange totalRange;
    totalRange.startRow = startRow;
    totalRange.startCol = startCol;
    totalRange.endRow = legendRange.endRow;
    totalRange.endCol = std::max(tableRange.endCol, legendRange.endCol);

    return totalRange;
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
    QString measureTitle = QString("POMIAR %1: %2 (ID: %3)").arg(index).arg(measurement->getSample()->getName()).arg(measurement->getId());    

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



