#include "analysis_measures_dialog.h"
#include "ui_analysis_measures_dialog.h"
#include "../../../radwag/measure_statistic_analyzer.h"
#include <qdialog.h>
#include <qscreen.h>
#include <qstandardpaths.h>
#include <QFileDialog>
#include <QMessageBox>
#include "../../../utils.h"

AnalysisMeasuresDialog::AnalysisMeasuresDialog(const std::vector<std::shared_ptr<const Measurement>>& measurements, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AnalysisMeasuresDialog)
    , results(AnalysisResult(measurements))
    // , densityResult(results.getDensityResult())
    // , porosityResult(results.getPorosityResult())
    , measurements(results.getMeasurements())
{
    results.getDensityResult();

    ui->setupUi(this);
    setWindowTitle("Analiza statystyczna gęstości pozornej");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initialize();
    updateView();
}

AnalysisMeasuresDialog::~AnalysisMeasuresDialog()
{
    delete ui;
}

void AnalysisMeasuresDialog::updateView()
{
    const auto &densityResult = results.getDensityResult();
    ui->editMeasuresSeries->setText(densityResult.getSeriesName());
    ui->editSamplesMaterial->setText(densityResult.getMaterialName());
    ui->editMaterialDenisty->setText(QString::number(densityResult.getTheoreticalDensity()) + "g/cm3");
    ui->lineEditDensityVariationCoeff->setText(QString("%1%").arg(densityResult.getVariationCoefficient(), 0, 'f', 2));

    ui->lineEditDensityMean->setText(QString("%1 g/cm³").arg(densityResult.getMean(), 0, 'f', 3));
    ui->lineEditDensityStdDev->setText(QString("%1 g/cm³").arg(densityResult.getStandardDeviation(), 0, 'f', 4));
    ui->lineEditDensityUncertainty->setText(QString("±%1 g/cm³").arg(densityResult.getUncertainty(), 0, 'f', 3));
    ui->lineEditDensityFinalResult->setText(densityResult.getFinalResult());

    const auto &porosityResult = results.getPorosityResult();
    ui->lineEditPorosityMean->setText(QString("%1 %").arg(porosityResult.getMean(), 0, 'f', 2));
    ui->lineEditPorosityStdDev->setText(QString("%1 %").arg(porosityResult.getStandardDeviation(), 0, 'f', 3));
    ui->lineEditPorosityVariationCoeff->setText(QString("%1%").arg(porosityResult.getVariationCoefficient(), 0, 'f', 2));
    ui->lineEditPorosityUncertainty->setText(QString("±%1 %").arg(porosityResult.getUncertainty(), 0, 'f', 2));
    ui->lineEditPorosityFinalResult->setText(porosityResult.getFinalResult());
}

const AnalysisResult &AnalysisMeasuresDialog::getAnalysisResult() const
{
    return results;
}

void AnalysisMeasuresDialog::onConfidenceLevelChanged(int index)
{
    double newConfidence = ui->comboBoxConfidence->itemData(index).toDouble();

    results.setConfidenceLevel(newConfidence);
    updateView();
}

void AnalysisMeasuresDialog::onSaveAnalysisButtonClicked()
{
    emit saveAnalysis();
}

void AnalysisMeasuresDialog::onExportPDFButtonClicked()
{
    QMessageBox::information(this, "Funkcja w rozwoju", "Eksport PDF będzie dostępny w przyszłej wersji.");
}

void AnalysisMeasuresDialog::onExportExcelButtonClicked()
{
    QMessageBox::information(this, "Funkcja w rozwoju", "Eksport Excel będzie dostępny w przyszłej wersji.");
}

void AnalysisMeasuresDialog::onAnalysisSaved()
{
    QMessageBox::information(this, "Zapis", "Analiza została pomyślnie zapisana");
}

void AnalysisMeasuresDialog::initialize()
{
    fillSelectedSamplesTable();
    fillConfidenceComboBox();
    connectSignals();
}

void AnalysisMeasuresDialog::fillSelectedSamplesTable()
{
    auto &measures = results.getMeasurements();
    ui->tableSelectedSamples->setRowCount(measures.size());

    for(int i = 0; i < measures.size(); i++)
    {
        auto &measure = measures[i];
        ui->tableSelectedSamples->setItem(i, 0, new QTableWidgetItem(measure->isThreeType() ? "Trzystopniowy" : "Dwustopniowy"));
        ui->tableSelectedSamples->setItem(i, 1, new QTableWidgetItem(utils::getSaturationMethodName(measure->getSaturationMethod())));
        ui->tableSelectedSamples->setItem(i, 2, new QTableWidgetItem(QString::number(measure->getResults().getApparentDensity())));
        ui->tableSelectedSamples->setItem(i, 3, new QTableWidgetItem(QString::number(measure->getResults().getTotalPorosity())));
        ui->tableSelectedSamples->setItem(i, 4, new QTableWidgetItem(measure->getAuthor()));
        ui->tableSelectedSamples->setItem(i, 5, new QTableWidgetItem(measure->getDate().toString("dd-MM-yyyy")));
        ui->tableSelectedSamples->setItem(i, 6, new QTableWidgetItem(measure->getId()));
    }

    ui->tableSelectedSamples->resizeColumnsToContents();
    ui->lineEditSampleCount->setText(QString::number(measures.size()));
}

void AnalysisMeasuresDialog::fillConfidenceComboBox()
{
    ui->comboBoxConfidence->clear();

    ui->comboBoxConfidence->addItem("90% (t₀.₁₀)", 0.90);
    ui->comboBoxConfidence->setItemData(0, 0.10, Qt::UserRole + 1);

    ui->comboBoxConfidence->addItem("95% (t₀.₀₅)", 0.95);
    ui->comboBoxConfidence->setItemData(0, 0.05, Qt::UserRole + 1);

    ui->comboBoxConfidence->addItem("99% (t₀.₀₁)", 0.99);
    ui->comboBoxConfidence->setItemData(1, 0.01, Qt::UserRole + 1);

    ui->comboBoxConfidence->setCurrentIndex(1);
}

void AnalysisMeasuresDialog::connectSignals()
{
    // connect(ui->buttonClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->buttonSaveAnalysis, &QPushButton::clicked, this, &AnalysisMeasuresDialog::onSaveAnalysisButtonClicked);
    connect(ui->buttonExportPDF, &QPushButton::clicked, this, &AnalysisMeasuresDialog::onExportPDFButtonClicked);
    connect(ui->buttonExportExcel, &QPushButton::clicked, this, &AnalysisMeasuresDialog::onExportExcelButtonClicked);
    connect(ui->comboBoxConfidence, &QComboBox::currentIndexChanged, this, &AnalysisMeasuresDialog::onConfidenceLevelChanged);
}
