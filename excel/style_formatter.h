#ifndef STYLE_FORMATTER_H
#define STYLE_FORMATTER_H

#include <xlsxdocument.h>
#include <xlsxformat.h>
#include <QColor>
#include <QFont>
#include <QPair>
#include <QPoint>


class StyleFormatter
{
private:
    QXlsx::Document* m_document;

    bool applyFormatToCell(int row, int col, const QXlsx::Format& formatToAdd);

public:
    explicit StyleFormatter(QXlsx::Document* doc);

    bool setCellBackground(int row, int col, const QColor& color);
    bool setCellFont(int row, int col, const QFont& font);
    bool setCellBorder(int row, int col, QXlsx::Format::BorderStyle style);
    bool setCellAlignment(int row, int col, QXlsx::Format::HorizontalAlignment hAlign, QXlsx::Format::VerticalAlignment vAlign);
    bool setCellFormat(int row, int col, const QXlsx::Format& format);

    bool setRangeBackground(int startRow, int startCol, int endRow, int endCol, const QColor& color);
    bool setRangeFont(int startRow, int startCol, int endRow, int endCol, const QFont& font);
    bool setRangeBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style);
    bool setRangeFormat(int startRow, int startCol, int endRow, int endCol, const QXlsx::Format& format);

    bool addBottomLine(int row, int startCol, int endCol, QXlsx::Format::BorderStyle style = QXlsx::Format::BorderThin, const QColor& color = Qt::black);
    bool addTopLine(int row, int startCol, int endCol, QXlsx::Format::BorderStyle style = QXlsx::Format::BorderThin, const QColor& color = Qt::black);
    bool addVerticalLine(int col, int startRow, int endRow, bool leftSide, QXlsx::Format::BorderStyle style = QXlsx::Format::BorderThin, const QColor& color = Qt::black);

    bool setOutlineBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style);
    bool setGridBorder(int startRow, int startCol, int endRow, int endCol, QXlsx::Format::BorderStyle style);

    bool setRowBackground(int row, int startCol, int endCol, const QColor& color);
    bool setColumnBackground(int col, int startRow, int endRow, const QColor& color);
    bool setAlternatingRowsBackground(int startRow, int endRow, int startCol, int endCol, const QColor& color1, const QColor& color2);

    bool clearFormat(int row, int col);
    bool clearRangeFormat(int startRow, int startCol, int endRow, int endCol);
    bool copyFormat(int fromRow, int fromCol, int toRow, int toCol);
};

#endif // STYLE_FORMATTER_H
