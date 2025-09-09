#include "section_excel.h"
#include "measurement_exporter.h"

Section::Section(const QString& title, const QXlsx::Format &titleFormat, MeasurementExporter *exporter, int startRow, int startCol)
    : m_title(title)
    , m_startRow(startRow)
    , m_startCol(startCol)
    , m_exporter(exporter)
    , m_titleFormat(titleFormat)
{

}

void Section::setColumnFormat(int col, const QXlsx::Format& format)
{
    if(col >= 0)
        m_columnFormats[col] = format;
}

void Section::setRowFormat(int row, const QXlsx::Format& format)
{
    if(row >= 0)
        m_rowFormats[row] = format;
}

void Section::setCellFormat(int row, int col, const QXlsx::Format& format)
{
    if(!isValidPosition(row, col))
        return;

    QPair<int, int> cellKey(row, col);
    m_cellFormats[cellKey] = format;
}

QXlsx::Format Section::getFormat(int row, int col) const
{
    if(!isValidPosition(row, col))
        return QXlsx::Format();

    QPair<int, int> cellKey(row, col);

    if(m_cellFormats.contains(cellKey))
        return m_cellFormats[cellKey];

    if(m_rowFormats.contains(row))
        return m_rowFormats[row];

    if(m_columnFormats.contains(col))
        return m_columnFormats[col];

    return QXlsx::Format();
}

int Section::addRow(const QVariantList& rowData)
{
    if(rowData.isEmpty())
        return -1;
    m_data.append(rowData);
    return m_data.size() - 1;
}

void Section::setCellValue(int row, int col, const QVariant& value)
{
    if(!isValidPosition(row, col))
        return;

    while (m_data.size() <= row)
        m_data.append(QVariantList());

    while (m_data[row].size() <= col)
        m_data[row].append(QVariant());

    m_data[row][col] = value;
}

QVariant Section::getCellValue(int row, int col) const
{
    if(isValidPosition(row, col) && row < m_data.size() && col < m_data[row].size())
        return m_data[row][col];
    return QVariant();
}

int Section::getColumnCount() const
{
    int maxCols = 0;
    for(const QVariantList& row : m_data)
        maxCols = qMax(maxCols, row.size());
    return maxCols;
}

void Section::render()
{
    if(!m_exporter || m_data.isEmpty())
        return;

    m_exporter->moveToPosition(m_startRow, 1);

    QVector<int> columnWidths = calculateColumnWidths();

    if(!m_title.isEmpty())
    {
        int totalWidth = 0;
        for(int width : columnWidths)
            totalWidth += width;
        renderHeader(totalWidth);
    }

    renderData(columnWidths);
    m_exporter->addEmptyRows(1);
    m_createdColumns = columnWidths;
}

QVector<int> Section::calculateColumnWidths() const
{
    int columnCount = getColumnCount();
    QVector<int> columnWidths(columnCount, 1);

    if(!m_exporter)
        return columnWidths;

    for(int col = 0; col < columnCount; col++)
    {
        int maxWidth = 1;

        for(int row = 0; row < m_data.size(); row++)
        {
            if(col < m_data[row].size())
            {
                QVariant cellValue = m_data[row][col];
                QXlsx::Format cellFormat = getFormat(row, col);

                QString text = cellValue.toString();
                int requiredWidth = m_exporter->calculateRequiredMergeCells(text, cellFormat.font());
                maxWidth = qMax(maxWidth, requiredWidth);
            }
        }
        columnWidths[col] = maxWidth;
    }
    return columnWidths;
}

const QVector<int>& Section::getCreatedColumns() const
{
    return m_createdColumns;
}

int Section::getTotalWidth() const
{
    const QVector<int>& widths = getCreatedColumns();
    if(widths.isEmpty())
        return 0;
    return std::accumulate(widths.begin(), widths.end(), 0);
}

SectionRange Section::getRange() const
{
    SectionRange r;
    r.startRow = m_startRow;
    r.startCol = m_startCol;
    r.endRow   = m_startRow + m_data.size() - 1;
    r.endCol   = m_startCol + getTotalWidth() - 1;
    return r;
}

void Section::renderHeader(int totalWidth)
{
    if(!m_exporter || m_title.isEmpty())
        return;
    m_exporter->writeValue(m_startRow, m_startCol, m_title, m_titleFormat);
    if(totalWidth > 1)
        m_exporter->mergeCells(m_startRow, m_startCol, m_startRow, m_startCol + totalWidth - 1);
    m_exporter->moveToNextRow();
}

void Section::renderData(const QVector<int>& columnWidths)
{
    if(!m_exporter)
        return;

    for(int row = 0; row < m_data.size(); row++)
    {
        int currentCol = m_startCol;

        for(int col = 0; col < m_data[row].size() && col < columnWidths.size(); col++)
        {
            QVariant cellValue = m_data[row][col];
            QXlsx::Format cellFormat = getFormat(row, col);
            int mergeWidth = columnWidths[col];

            m_exporter->writeValue(m_exporter->getCurrentRow(), currentCol, cellValue, cellFormat);

            if(mergeWidth > 1)
                m_exporter->mergeCells(m_exporter->getCurrentRow(), currentCol, m_exporter->getCurrentRow(), currentCol + mergeWidth - 1);

            currentCol += mergeWidth;
        }

        m_exporter->moveToNextRow();
    }
}

bool Section::isValidPosition(int row, int col) const
{
    return row >= 0 && col >= 0;
}

// void Section::addSectionHeader(const QString& title, int col, int columnSpan)
// {
//     int currentRow = m_exporter->getCurrentRow();
//     m_exporter->writeValue(currentRow, 1, title, m_sectionTitleFormat);
//     if(columnSpan > 1)
//         m_exporter->mergeCells(currentRow, 1, currentRow, columnSpan);
//     m_exporter->moveToNextRow();
// }


