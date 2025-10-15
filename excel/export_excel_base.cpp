#include "export_excel_base.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFontMetrics>
#include <QApplication>
#include <qregularexpression.h>

ExportExcelBase::ExportExcelBase(const QString& fileName)
    : m_document(nullptr)
    , m_filePath(fileName)
    , m_currentRow(1)
    , m_currentColumn(1)
{
    initializeDocument();
}

ExportExcelBase::~ExportExcelBase()
{
    delete m_document;
}

void ExportExcelBase::initializeDocument()
{
    m_document = new QXlsx::Document();
    if(!m_document)
        return;

    m_document->setDocumentProperty("creator", "SerialPortConnectorRadwag");
    m_document->setDocumentProperty("title", "Measurement Export");
}

bool ExportExcelBase::saveDocument()
{
    if(!m_document)
        return false;

    QFileInfo fileInfo(m_filePath);
    QDir dir = fileInfo.absoluteDir();
    if(!dir.exists())
        dir.mkpath(".");

    return m_document->saveAs(m_filePath);
}

QString ExportExcelBase::getFilePath() const
{
    return m_filePath;
}

std::shared_ptr<QXlsx::Cell> ExportExcelBase::getCell(int row, int col) const
{
    if(!m_document || !isValidPosition(row, col))
        return nullptr;
    return m_document->currentWorksheet()->cellAt(row, col);
}

void ExportExcelBase::writeValue(int row, int col, const QVariant& value)
{
    if(m_document && isValidPosition(row, col))
        m_document->write(row, col, value);
}

void ExportExcelBase::writeValue(int row, int col, const QVariant& value, const QXlsx::Format& format)
{
    if(m_document && isValidPosition(row, col))
        m_document->write(row, col, value, format);
}

QVariant ExportExcelBase::readValue(int row, int col) const
{
    if(m_document && isValidPosition(row, col))
        return m_document->read(row, col);
    return QVariant();
}

void ExportExcelBase::writeRow(int row, const QVariantList& data, int startColumn)
{
    for(int i = 0; i < data.size(); i++)
        writeValue(row, startColumn + i, data[i]);
}

void ExportExcelBase::writeColumn(int column, const QVariantList& data, int startRow)
{
    for(int i = 0; i < data.size(); i++)
        writeValue(startRow + i, column, data[i]);
}

void ExportExcelBase::addEmptyRows(int count)
{
    m_currentRow += count;
}

QVariantList ExportExcelBase::readRow(int row, int fromColumn, int toColumn) const
{
    QVariantList result;
    for(int col = fromColumn; col <= toColumn; col++)
        result.append(readValue(row, col));
    return result;
}

QVariantList ExportExcelBase::readColumn(int column, int fromRow, int toRow) const
{
    QVariantList result;
    for(int row = fromRow; row <= toRow; row++)
        result.append(readValue(row, column));
    return result;
}

void ExportExcelBase::setColumnWidth(int column, double width)
{
    if(m_document && column > 0)
        m_document->setColumnWidth(column, width);
}

void ExportExcelBase::setColumnWidths(int fromColumn, int toColumn, double width)
{
    for(int col = fromColumn; col <= toColumn; ++col)
        setColumnWidth(col, width);
}

double ExportExcelBase::getColumnWidth(int column) const
{
    if(m_document && column > 0)
        return m_document->columnWidth(column);
    return COL_WIDTH;
}

void ExportExcelBase::setRowHeight(int row, double height)
{
    if(m_document && row > 0)
        m_document->setRowHeight(row, height);
}

void ExportExcelBase::setRowHeights(int fromRow, int toRow, double height)
{
    for(int row = fromRow; row <= toRow; ++row)
        setRowHeight(row, height);
}

double ExportExcelBase::getRowHeight(int row) const
{
    if(m_document && row > 0)
        return m_document->rowHeight(row);
    return 15.0; // Domyślna wysokość wiersza w Excel
}

double ExportExcelBase::calculateTextWidth(const QString& text, const QFont& font) const
{
    QFont usedFont = font.family().isEmpty() ? QFont("Calibri", 10) : font;
    QFontMetrics metrics(usedFont);

    int pixelWidth = metrics.horizontalAdvance(text);
    double excelUnits = pixelWidth / 7.0;

    return excelUnits + 1.0;
}

int ExportExcelBase::calculateRequiredCells(const QString& text, const QFont& font, double cellWidth) const
{
    double textWidthInExcelUnits = calculateTextWidth(text, font);
    return qMax(1, qCeil(textWidthInExcelUnits / cellWidth));
}

int ExportExcelBase::calculateRequiredMergeCells(const QString& text, const QFont& font, double cellWidth) const
{
    double textWidth = calculateTextWidth(text, font);
    double actualColumnWidth = cellWidth <= 0 ? getColumnWidth(getCurrentColumn()) : cellWidth;
    int requiredCells = qMax(1, qCeil(textWidth / actualColumnWidth));
    return requiredCells;
}

void ExportExcelBase::mergeCells(int fromRow, int fromCol, int toRow, int toCol)
{
    if(m_document && isValidRange(fromRow, fromCol, toRow, toCol))
    {
        QXlsx::CellRange range(fromRow, fromCol, toRow, toCol);
        m_document->mergeCells(range);
    }
}

void ExportExcelBase::mergeCells(const QString& range)
{
    if(m_document)
    {
        QXlsx::CellRange cellRange(range);
        m_document->mergeCells(cellRange);
    }
}

void ExportExcelBase::unmergeCells(int fromRow, int fromCol, int toRow, int toCol)
{
    if(m_document && isValidRange(fromRow, fromCol, toRow, toCol))
    {
        QXlsx::CellRange range(fromRow, fromCol, toRow, toCol);
        m_document->unmergeCells(range);
    }
}

void ExportExcelBase::copyRange(int fromRow, int fromCol, int toRow, int toCol, int destRow, int destCol)
{
    if(!isValidRange(fromRow, fromCol, toRow, toCol) || !isValidPosition(destRow, destCol))
        return;

    for(int row = fromRow; row <= toRow; ++row)
    {
        for(int col = fromCol; col <= toCol; ++col)
        {
            QVariant value = readValue(row, col);
            int newRow = destRow + (row - fromRow);
            int newCol = destCol + (col - fromCol);
            writeValue(newRow, newCol, value);
        }
    }
}

void ExportExcelBase::clearRange(int fromRow, int fromCol, int toRow, int toCol)
{
    if(!isValidRange(fromRow, fromCol, toRow, toCol))
        return;

    for(int row = fromRow; row <= toRow; row++)
        for(int col = fromCol; col <= toCol; col++)
            writeValue(row, col, QVariant());
}

bool ExportExcelBase::isCellEmpty(int row, int col) const
{
    QVariant value = readValue(row, col);
    return value.isNull() || value.toString().trimmed().isEmpty();
}

bool ExportExcelBase::isRangeEmpty(int fromRow, int fromCol, int toRow, int toCol) const
{
    for(int row = fromRow; row <= toRow; row++)
        for(int col = fromCol; col <= toCol; col++)
            if(!isCellEmpty(row, col))
                return false;
    return true;
}

QString ExportExcelBase::cellToString(int row, int col) const
{
    QString colStr;
    int tempCol = col - 1;
    while(tempCol >= 0)
    {
        colStr = QChar('A' + (tempCol % 26)) + colStr;
        tempCol = tempCol / 26 - 1;
    }
    return colStr + QString::number(row);
}

QPair<int,int> ExportExcelBase::stringToCell(const QString& cell) const
{
    QRegularExpression re("^([A-Z]+)(\\d+)$");
    QRegularExpressionMatch match = re.match(cell.toUpper());

    if(!match.hasMatch())
        return QPair<int,int>(1, 1);

    QString colStr = match.captured(1);
    int row = match.captured(2).toInt();

    int col = 0;
    for (QChar c : colStr)
        col = col * 26 + (c.toLatin1() - 'A' + 1);

    return QPair<int,int>(row, col);
}

bool ExportExcelBase::isValidPosition(int row, int col) const
{
    return row > 0 && col > 0 && row <= 1048576 && col <= 16384; // Limit Excel
}

bool ExportExcelBase::isValidRange(int fromRow, int fromCol, int toRow, int toCol) const
{
    return isValidPosition(fromRow, fromCol) && isValidPosition(toRow, toCol) && fromRow <= toRow && fromCol <= toCol;
}

void ExportExcelBase::moveToPosition(int row, int col)
{
    if(isValidPosition(row, col))
    {
        m_currentRow = row;
        m_currentColumn = col;
    }
}

void ExportExcelBase::moveToNextRow()
{
    m_currentRow++;
}

void ExportExcelBase::moveToNextColumn()
{
    m_currentColumn++;
}

void ExportExcelBase::moveRows(int count)
{
    m_currentRow += count;
}

void ExportExcelBase::moveColumns(int count)
{
    m_currentColumn += count;
}

int ExportExcelBase::getCurrentRow() const
{
    return m_currentRow;
}

int ExportExcelBase::getCurrentColumn() const
{
    return m_currentColumn;
}

void ExportExcelBase::printCurrentPosition() const
{
    qDebug() << "Current position:" << cellToString(m_currentRow, m_currentColumn) << "(" << m_currentRow << "," << m_currentColumn << ")";
}

QXlsx::Document* ExportExcelBase::getDocument() const
{
    return m_document;
}

