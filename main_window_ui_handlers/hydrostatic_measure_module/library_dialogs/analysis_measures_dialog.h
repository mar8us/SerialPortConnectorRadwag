#ifndef ANALYSIS_MEASURES_DIALOG_H
#define ANALYSIS_MEASURES_DIALOG_H

#include <QMainWindow>
#include <qabstractitemmodel.h>
#include "../../../radwag/measurement.h"
#include "../../../radwag/measure_statistic_analyzer.h"
#include <QStyledItemDelegate>


namespace Ui {
class AnalysisMeasuresDialog;
}


enum TableColumns
{
    COL_PARAMETER,
    COL_FIRST_SAMPLE,
};

enum StatisticTableColmuns
{
    STAT_MEAN_OFFSET,
    STAT_STDDEV_OFFSET,
    STAT_CV_OFFSET,
    STAT_UNCERTAINTY_OFFSET,
    STAT_SUMMARY_OFFSET,
    STAT_COL_COUNT
};

class MeasurementTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MeasurementTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setAnalysisResults(const AnalysisResult& results);
    void updateResults();
    void clear();

    QVariant getDisplayData(const QModelIndex& index, const BaseAnalysisResult& result) const;

private:
    struct ResultNode
    {
        const BaseAnalysisResult* analysisResult;
        QString parameterName;

        ResultNode(const BaseAnalysisResult* result, const QString& name)
            : analysisResult(result), parameterName(name)
        { }
    };

    std::vector<std::unique_ptr<ResultNode>> nodes;
    int sampleCount;

    int getSampleColumnIndex(int sampleIndex) const;
    int getStatColumnIndex(int statOffset) const;
    QString formatValue(double value, int precision) const;
    int getPrecisionForValues(const BaseAnalysisResult& result) const;
};


class AnalysisMeasuresDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnalysisMeasuresDialog(const std::vector<std::shared_ptr<const Measurement>>& measurements, QWidget *parent = nullptr);
    ~AnalysisMeasuresDialog();

    void updateView();
    const AnalysisResult &getAnalysisResult() const;

signals:
    void exportAnalysisExcel(const AnalysisResult *results);

private slots:
    void onConfidenceLevelChanged(int index);
    void onExportExcelButtonClicked();

private:
    void initialize();
    void setupTableView();
    void fillConfidenceComboBox();

    void connectSignals();

    Ui::AnalysisMeasuresDialog *ui;

    MeasurementTableModel* tableModel;
    AnalysisResult *results;
    const std::vector<std::shared_ptr<const Measurement>>& measurements;
    // const DensityAnalysisResult &densityResult;
    // const PorosityAnalysisResult &porosityResult;
};

#endif // ANALYSIS_MEASURES_DIALOG_H
