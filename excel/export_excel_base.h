#ifndef EXPORTEXCELBASE_H
#define EXPORTEXCELBASE_H

#include <QString>
#include <QVariant>
#include <QStringList>
#include <QVariantList>
#include <QSize>
#include <QPair>
#include <QColor>
#include <QFont>
#include <xlsxdocument.h>
#include <xlsxformat.h>

const static double COL_WIDTH = 10.0;

class ExportExcelBase
{
public:
    explicit ExportExcelBase(const QString& fileName);
    virtual ~ExportExcelBase();

    virtual bool exportData() = 0;

    bool saveDocument();
    QString getFilePath() const;

    std::shared_ptr<QXlsx::Cell> getCell(int row, int col) const;
    void writeValue(int row, int col, const QVariant& value);
    void writeValue(int row, int col, const QVariant& value, const QXlsx::Format& format);
    QVariant readValue(int row, int col) const;

    void writeRow(int row, const QVariantList& data, int startColumn = 1);
    void writeColumn(int column, const QVariantList& data, int startRow = 1);
    void addEmptyRows(int count = 1);
    QVariantList readRow(int row, int fromColumn, int toColumn) const;
    QVariantList readColumn(int column, int fromRow, int toRow) const;

    void setColumnWidth(int column, double width);
    void setColumnWidths(int fromColumn, int toColumn, double width);
    double getColumnWidth(int column) const;

    double calculateTextWidth(const QString& text, const QFont& font = QFont()) const;
    int calculateRequiredCells(const QString& text, const QFont& font = QFont(), double cellWidth = COL_WIDTH) const;

    int calculateRequiredMergeCells(const QString& text, const QFont& font = QFont(), double cellWidth = COL_WIDTH) const;

    void mergeCells(int fromRow, int fromCol, int toRow, int toCol);
    void mergeCells(const QString& range); // np. "A1:C1"
    void unmergeCells(int fromRow, int fromCol, int toRow, int toCol);

    void copyRange(int fromRow, int fromCol, int toRow, int toCol, int destRow, int destCol);
    void clearRange(int fromRow, int fromCol, int toRow, int toCol);
    bool isCellEmpty(int row, int col) const;
    bool isRangeEmpty(int fromRow, int fromCol, int toRow, int toCol) const;

    QString cellToString(int row, int col) const;
    QPair<int,int> stringToCell(const QString& cell) const;

    bool isValidPosition(int row, int col) const;
    bool isValidRange(int fromRow, int fromCol, int toRow, int toCol) const;

    void moveToPosition(int row, int col);
    void moveToNextRow();
    void moveToNextColumn();
    void moveRows(int count);
    void moveColumns(int count);
    int getCurrentRow() const;
    int getCurrentColumn() const;

    void printCurrentPosition() const;

protected:
    QXlsx::Document* getDocument() const;

private:
    void initializeDocument();

    QXlsx::Document* m_document;
    QString m_filePath;

    int m_currentRow;
    int m_currentColumn;
};

#endif // EXPORTEXCELBASE_H
