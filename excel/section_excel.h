#ifndef SECTION_EXCEL_H
#define SECTION_EXCEL_H

#include <QString>
#include <QVector>
#include <QVariantList>
#include <QMap>
#include <QPair>
#include "xlsxformat.h"

class MeasurementExporter;

struct SectionRange
{
    int startRow;
    int startCol;
    int endRow;
    int endCol;

    int rowCount() const
        {    return (endRow >= startRow) ? (endRow - startRow + 1) : 0;    }
    int colCount() const
        {    return (endCol >= startCol) ? (endCol - startCol + 1) : 0;    }

    bool isValid() const
        {    return startRow > 0 && startCol > 0 && endRow >= startRow && endCol >= startCol;   }
};

class Section
{
public:
    // Konstruktor
    Section(const QString& title, const QXlsx::Format& titleFormat, MeasurementExporter* exporter, int startRow = 1, int startCol = 1);

    // Formatowanie - hierarchia: Cell > Row > Column > Default
    void setColumnFormat(int col, const QXlsx::Format& format);
    void setRowFormat(int row, const QXlsx::Format& format);
    void setCellFormat(int row, int col, const QXlsx::Format& format);

    void setTitleFormat(const QXlsx::Format& format);

    QXlsx::Format getFormat(int row, int col) const;

    int addRow(const QVariantList& rowData);
    void setCellValue(int row, int col, const QVariant& value);
    QVariant getCellValue(int row, int col) const;

    void render();

    // Gettery
    int getRowCount() const { return m_data.size(); }
    int getColumnCount() const;
    QString getTitle() const { return m_title; }
    int getStartRow() const { return m_startRow; }
    int getStartCol() const { return m_startCol; }

    const QVector<int>& getCreatedColumns() const;
    int getTotalWidth() const;
    SectionRange getRange() const;

private:
    QVector<int> calculateColumnWidths() const;
    void renderHeader(int totalWidth);
    void renderData(const QVector<int>& columnWidths);
    bool isValidPosition(int row, int col) const;

    QString m_title;
    QVector<QVariantList> m_data;

    int m_startRow;
    int m_startCol;

    QVector<int> m_createdColumns;

    QMap<int, QXlsx::Format> m_columnFormats;
    QMap<int, QXlsx::Format> m_rowFormats;
    QMap<QPair<int,int>, QXlsx::Format> m_cellFormats;

    QXlsx::Format m_titleFormat;

    MeasurementExporter* m_exporter;
};

#endif
