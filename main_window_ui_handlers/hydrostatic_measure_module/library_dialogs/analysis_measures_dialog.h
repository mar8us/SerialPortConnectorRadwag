#ifndef ANALYSIS_MEASURES_DIALOG_H
#define ANALYSIS_MEASURES_DIALOG_H

#include <QMainWindow>
#include "../../../radwag/measurement.h"
#include "../../../radwag/measure_statistic_analyzer.h"

namespace Ui {
class AnalysisMeasuresDialog;
}

class AnalysisMeasuresDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnalysisMeasuresDialog(const std::vector<std::shared_ptr<const Measurement>>& measurements, QWidget *parent = nullptr);
    ~AnalysisMeasuresDialog();

    void updateView();
    const AnalysisResult &getAnalysisResult() const;

private slots:
    void onConfidenceLevelChanged(int index);

    void onSaveAnalysisButtonClicked();
    void onExportPDFButtonClicked();
    void onExportExcelButtonClicked();

    void onAnalysisSaved();

private:
    void initialize();
    void fillSelectedSamplesTable();
    void fillConfidenceComboBox();
    void connectSignals();

    Ui::AnalysisMeasuresDialog *ui;

    AnalysisResult results;
    // const DensityAnalysisResult &densityResult;
    // const PorosityAnalysisResult &porosityResult;
    const std::vector<std::shared_ptr<const Measurement>>& measurements;

signals:
    void saveAnalysis();
};

#endif // ANALYSIS_MEASURES_DIALOG_H
