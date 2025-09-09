#ifndef SUMMARY_MEASURE_DIALOG_H
#define SUMMARY_MEASURE_DIALOG_H

#include <QMainWindow>
#include "../../../radwag/measurement.h"
#include "../../../charts/porosity_chart.h"
#include "../../../charts/denisty_chart.h"

namespace Ui {
class SummaryMeasureDialog;
}

class SummaryMeasureDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit SummaryMeasureDialog(std::shared_ptr<const Measurement> sourceMeasure, QWidget *parent = nullptr);
    ~SummaryMeasureDialog();

signals:
    void exportMeasuresExcel(const QList<const Measurement*>& measures);

private:
    Ui::SummaryMeasureDialog *ui;

    void setupSecondCharts();
    void setupTripleCharts();

    void updateMeasureSecondLabelsSummary();
    void updateMeasureTripleLabelsSummary();

    void updateSecondCharts(const MeasurementResults &results);
    void updateTripleCharts(const MeasurementResults &results);

    std::shared_ptr<const Measurement> sourceMeasure;

    PorosityChartWidget *porosityChart;
    DensityChartWidget *denistyChart;
    DensityChartWidget *denistyChartSecond;

private slots:
    void onExcelExportButtonClicked();
};

#endif // SUMMARY_MEASURE_DIALOG_H
