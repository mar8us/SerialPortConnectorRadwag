// StyleFormatter.cpp
#include "style_formatter.h"
#include <QDebug>

StyleFormatter::StyleFormatter(QXlsx::Document* doc)
    : m_document(doc)
{

}

bool StyleFormatter::applyFormatToCell(int row, int col, const QXlsx::Format& formatToAdd)
{
    if(!m_document)
        return false;

    auto cell = m_document->cellAt(row, col);

    if(cell)
    {
        QVariant value = cell->value();

        if(cell->hasFormula())
            return false;

        QXlsx::Format currentFormat = cell->format();
        currentFormat.mergeFormat(formatToAdd);

        return m_document->write(row, col, value, currentFormat);
    }
    else
        return m_document->write(row, col, "", formatToAdd);
}


bool StyleFormatter::setCellBackground(int row, int col, const QColor& color)
{
    QXlsx::Format format;
    format.setPatternBackgroundColor(color);
    format.setFillPattern(QXlsx::Format::PatternSolid);
    return applyFormatToCell(row, col, format);
}

bool StyleFormatter::setCellFont(int row, int col, const QFont& font)
{
    QXlsx::Format format;
    format.setFontName(font.family());
    format.setFontSize(font.pointSize());
    format.setFontBold(font.bold());
    format.setFontItalic(font.italic());
    format.setFontUnderline(font.underline() ? QXlsx::Format::FontUnderlineSingle : QXlsx::Format::FontUnderlineNone);
    format.setFontStrikeOut(font.strikeOut());
    return applyFormatToCell(row, col, format);
}

bool StyleFormatter::setCellBorder(int row, int col, QXlsx::Format::BorderStyle style)
{
    QXlsx::Format format;
    format.setLeftBorderStyle(style);
    format.setRightBorderStyle(style);
    format.setTopBorderStyle(style);
    format.setBottomBorderStyle(style);
    format.setBorderColor(Qt::black);
    return applyFormatToCell(row, col, format);
}

bool StyleFormatter::setCellAlignment(int row, int col, QXlsx::Format::HorizontalAlignment hAlign, QXlsx::Format::VerticalAlignment vAlign)
{
    QXlsx::Format format;
    format.setHorizontalAlignment(hAlign);
    format.setVerticalAlignment(vAlign);
    return applyFormatToCell(row, col, format);
}

bool StyleFormatter::setCellFormat(int row, int col, const QXlsx::Format& format)
{
    return applyFormatToCell(row, col, format);
}


bool StyleFormatter::setRangeBackground(int startRow, int startCol, int endRow, int endCol, const QColor& color)
{
    if (!m_document)
        return false;

    bool success = true;
    for(int row = startRow; row <= endRow; row++)
        for(int col = startCol; col <= endCol; col++)
            if(!setCellBackground(row, col, color))
                success = false;
    return success;
}

bool StyleFormatter::setRangeFont(int startRow, int startCol, int endRow, int endCol, const QFont& font)
{
    if (!m_document)
        return false;

    bool success = true;
    for(int row = startRow; row <= endRow; row++)
        for(int col = startCol; col <= endCol; col++)
            if(!setCellFont(row, col, font))
                success = false;
    return success;
}

bool StyleFormatter::setRangeBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style)
{
    if (!m_document)
        return false;

    bool success = true;
    for(int row = startRow; row <= endRow; row++)
        for(int col = startCol; col <= endCol; col++)
            if(!setCellBorder(row, col, style))
                success = false;
    return success;
}

bool StyleFormatter::setRangeFormat(int startRow, int startCol, int endRow, int endCol, const QXlsx::Format& format)
{
    if (!m_document)
        return false;

    bool success = true;
    for(int row = startRow; row <= endRow; row++)
        for(int col = startCol; col <= endCol; col++)
            if(!setCellFormat(row, col, format))
                success = false;
    return success;
}

// === OBRAMOWANIA SPECJALNE ===

bool StyleFormatter::setOutlineBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style)
{
    if(!m_document)
        return false;

    bool success = true;

    for(int row = startRow; row <= endRow; row++)
    {
        for (int col = startCol; col <= endCol; col++)
        {
            QXlsx::Format format;

            if (row == startRow)
                format.setTopBorderStyle(style);
            if (row == endRow)
                format.setBottomBorderStyle(style);
            if (col == startCol)
                format.setLeftBorderStyle(style);
            if (col == endCol)
                format.setRightBorderStyle(style);

            if (row == startRow)
                format.setTopBorderColor(Qt::black);
            if (row == endRow)
                format.setBottomBorderColor(Qt::black);
            if (col == startCol)
                format.setLeftBorderColor(Qt::black);
            if (col == endCol)
                format.setRightBorderColor(Qt::black);

            if (!applyFormatToCell(row, col, format))
                success = false;
        }
    }
    return success;
}

bool StyleFormatter::addBottomLine(int row, int startCol, int endCol, QXlsx::Format::BorderStyle style, const QColor& color)
{
    if(!m_document)
        return false;

    bool success = true;

    for(int col = startCol; col <= endCol; col++)
    {
        QXlsx::Format format;
        format.setBottomBorderStyle(style);
        format.setBottomBorderColor(color);

        if(!applyFormatToCell(row, col, format))
            success = false;
    }

    return success;
}


bool StyleFormatter::addTopLine(int row, int startCol, int endCol, QXlsx::Format::BorderStyle style, const QColor& color)
{
    if(!m_document)
        return false;

    bool success = true;

    for(int col = startCol; col <= endCol; col++)
    {
        QXlsx::Format format;
        format.setTopBorderStyle(style);
        format.setTopBorderColor(color);

        if(!applyFormatToCell(row, col, format))
            success = false;
    }

    return success;
}

bool StyleFormatter::addVerticalLine(int col, int startRow, int endRow, bool leftSide, QXlsx::Format::BorderStyle style, const QColor& color)
{
    if(!m_document)
        return false;

    bool success = true;

    for(int row = startRow; row <= endRow; row++)
    {
        QXlsx::Format format;
        if(leftSide)
        {
            format.setLeftBorderStyle(style);
            format.setLeftBorderColor(color);
        }
        else
        {
            format.setRightBorderStyle(style);
            format.setRightBorderColor(color);
        }

        if(!applyFormatToCell(row, col, format))
            success = false;
    }
    return success;
}

bool StyleFormatter::setGridBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style)
{
    if(!m_document)
        return false;

    bool success = true;

    for(int row = startRow; row <= endRow; row++)
    {
        for(int col = startCol; col <= endCol; col++)
        {
            QXlsx::Format format;

            format.setLeftBorderStyle(style);
            format.setRightBorderStyle(style);
            format.setTopBorderStyle(style);
            format.setBottomBorderStyle(style);
            format.setBorderColor(Qt::black);

            if(!applyFormatToCell(row, col, format))
                success = false;
        }
    }
    return success;
}


bool StyleFormatter::setRowBackground(int row, int startCol, int endCol, const QColor& color)
{
    return setRangeBackground(row, startCol, row, endCol, color);
}

bool StyleFormatter::setColumnBackground(int col, int startRow, int endRow, const QColor& color)
{
    return setRangeBackground(startRow, col, endRow, col, color);
}

bool StyleFormatter::setAlternatingRowsBackground(int startRow, int endRow, int startCol, int endCol, const QColor& color1, const QColor& color2)
{
    if(!m_document)
        return false;

    bool success = true;
    for(int row = startRow; row <= endRow; row++)
    {
        QColor color = ((row - startRow) % 2 == 0) ? color1 : color2;
        if(!setRowBackground(row, startCol, endCol, color))
            success = false;
    }
    return success;
}

// === NARZĘDZIA ===

bool StyleFormatter::clearFormat(int row, int col)
{
    if(!m_document)
        return false;

    auto cell = m_document->cellAt(row, col);
    if(!cell)
        return false;

    QVariant value = cell->value();

    if(cell->hasFormula())
        return false;

    QXlsx::Format clearFormat;
    return m_document->write(row, col, value, clearFormat);
}

bool StyleFormatter::clearRangeFormat(int startRow, int startCol, int endRow, int endCol)
{
    if(!m_document)
        return false;

    bool success = true;
    for (int row = startRow; row <= endRow; row++)
        for(int col = startCol; col <= endCol; col++)
            if(!clearFormat(row, col))
                success = false;
    return success;
}

bool StyleFormatter::copyFormat(int fromRow, int fromCol, int toRow, int toCol)
{
    if(!m_document)
        return false;

    auto sourceCell = m_document->cellAt(fromRow, fromCol);
    if(!sourceCell)
        return false;

    QXlsx::Format sourceFormat = sourceCell->format();

    auto targetCell = m_document->cellAt(toRow, toCol);
    if(targetCell)
    {
        QVariant targetValue = targetCell->value();
        if(targetCell->hasFormula())
            return false;
        return m_document->write(toRow, toCol, targetValue, sourceFormat);
    }
    else
        return m_document->write(toRow, toCol, "", sourceFormat);
}
