#ifndef MEASUREMENT_EXPORTER_H
#define MEASUREMENT_EXPORTER_H

#include "export_excel_base.h"
#include "../radwag/measurement.h"
#include "section_excel.h"
#include <QList>
#include <QString>

class MeasurementExporter : public ExportExcelBase
{
public:
    explicit MeasurementExporter(const QString& fileName, const Measurement* measurement);
    explicit MeasurementExporter(const QString& fileName, const QList<const Measurement*>& measurements);

    bool exportData() override;

private:
    QList<const Measurement*> m_measurements;

    SectionRange exportGeneralDataSection(const Measurement* measurement, int startRow, int startCol);
    SectionRange exportMeasurementDataSection(const Measurement* measurement, int startRow, int startCol);
    SectionRange exportCalculationResultsSection(const Measurement* measurement, int startRow, int startCol);
    SectionRange exportCommentsSection(const Measurement* measurement); //TO DO

    SectionRange addMeasurementTitle(const Measurement* measurement, int index, int startRow, int startCol);

    void setupColumnSize(int columns, int size = 10);
    void setupStandardSectionFormat(Section& section);

    SectionRange exportMeasurementTableSection(const QList<const Measurement *> &measurements, int startRow, int startCol);

    bool hasValidResults(const Measurement* measurement) const;
    bool validateData() const;

private:
    QXlsx::Format createFormat(bool bold = false, int fontSize = 10, QXlsx::Format::HorizontalAlignment hAlign = QXlsx::Format::AlignHGeneral,
                               const QColor& backgroundColor = QColor(), QXlsx::Format::BorderStyle borderStyle = QXlsx::Format::BorderNone) const;

    void initializeFormats() const;
    QXlsx::Format getTitleFormat() const;
    QXlsx::Format getSectionTitleFormat() const;
    QXlsx::Format getKeyFormat() const;
    QXlsx::Format getValueFormat() const;
    QXlsx::Format getDecimalFormat(int precision = 4) const;
    QXlsx::Format getIntegerFormat() const;
    QXlsx::Format getPercentageFormat() const;
    QXlsx::Format getPercentageFormat(int precision = 2) const;
    QXlsx::Format getDateTimeFormat() const;
    QXlsx::Format getDateFormat() const;
    QXlsx::Format getUnitFormat() const;
    QXlsx::Format getTableHeaderFormat() const;

    mutable bool m_formatsInitialized;
    mutable QXlsx::Format m_titleFormat;
    mutable QXlsx::Format m_sectionTitleFormat;
    mutable QXlsx::Format m_keyFormat;
    mutable QXlsx::Format m_valueFormat;
    mutable QXlsx::Format m_decimalFormat;
    mutable QXlsx::Format m_integerFormat;
    mutable QXlsx::Format m_percentageFormat;
    mutable QXlsx::Format m_dateTimeFormat;
    mutable QXlsx::Format m_dateFormat;
    mutable QXlsx::Format m_unitFormat;
    mutable QXlsx::Format m_tableHeaderFormat;

    static const QColor LIGHT_BLUE;
    static const QColor LIGHT_GRAY;
    static const QColor LIGHT_GRAY_H;
    static const QColor UNIT_FONT;
};

#endif // MEASUREMENTEXPORTER_H
