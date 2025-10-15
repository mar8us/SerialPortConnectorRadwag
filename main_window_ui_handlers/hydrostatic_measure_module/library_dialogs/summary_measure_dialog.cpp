#include "summary_measure_dialog.h"
#include "ui_summary_measure_dialog.h"
#include "../../../utils.h"

SummaryMeasureDialog::SummaryMeasureDialog(std::shared_ptr<const Measurement> sourceMeasure, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SummaryMeasureDialog)
    , sourceMeasure(sourceMeasure)
    , porosityChart(nullptr)
    , denistyChart(nullptr)
    , denistyChartSecond(nullptr)
{
    ui->setupUi(this);
    if(sourceMeasure->isThreeType())
    {
        setupTripleCharts();
        ui->stackedWidget->setCurrentIndex(1);
        updateMeasureTripleLabelsSummary();
    }
    else
    {
        setupSecondCharts();
        ui->stackedWidget->setCurrentIndex(0);
        updateMeasureSecondLabelsSummary();
    }

    connect(ui->buttonExcelExport, &QPushButton::clicked, this, &SummaryMeasureDialog::onExcelExportButtonClicked);
}

SummaryMeasureDialog::~SummaryMeasureDialog()
{
    delete ui;
}

void SummaryMeasureDialog::updateMeasureSecondLabelsSummary()
{
    if(!sourceMeasure || !sourceMeasure->hasResults())
        return;

    auto results = sourceMeasure->getResults();

    ui->valueSecondMeasureType->setText(sourceMeasure->getType() == MeasurementType::TwoStage ? "Dwustopniowy" : "Trzystopniowy");
    ui->valueSecondMeasureOperator->setText(sourceMeasure->getAuthor());
    QString dateTime = sourceMeasure->getDate().toString("dd-MM-yyyy HH:mm");
    ui->valueSecondMeasureDateTime->setText(dateTime);
    QString endDateTime = sourceMeasure->getEndDate().toString("dd-MM-yyyy HH:mm");
    ui->valueSecondMeasureEndDateTime->setText(endDateTime);

    auto sample = sourceMeasure->getSample();
    ui->valueSecondSampleName->setText(sample->getName());
    ui->valueSecondMaterial->setText(sample->getMaterialName());
    ui->valueSecondTheoreticalDensity->setText(QString::number(sample->getMaterialDensity()) + " g/cm³");

    Fluid fluid = sourceMeasure->getFluid();
    ui->valueSecondLiquidType->setText(fluid.getName());
    ui->valueSecondFluidTemp->setText(QString::number(sourceMeasure->getFluidTemperature()) + " °C");
    ui->valueSecondLiquidDensity->setText(QString::number(sourceMeasure->getFluidDensity(), 'f', 5) + " g/cm³");

    double dryMass = sourceMeasure->getSampleDryMass();
    ui->valueSecondMeasureDryMass->setText(QString::number(dryMass) + " g");
    double massInFluid = sourceMeasure->getSampleInFluidMass();
    ui->valueSecondMeasureWetMass->setText(QString::number(massInFluid) + " g");

    double apparentVolume = results.getApparentVolume();
    ui->valueSecondMeasureApparentVolume->setText(QString::number(apparentVolume) + " cm³");
    double apparentDensity = results.getApparentDensity();
    ui->valueSecondMeasureApparentDensity->setText(QString::number(apparentDensity) + " g/cm³");
    double relativeDensity = results.getRelativeDensity();
    ui->valueSecondMeasureRelativeDensity->setText(QString::number(relativeDensity) + " %");
    double totalPorosity = results.getTotalPorosity();
    ui->valueSecondMeasureTotalPorosity->setText(QString::number(totalPorosity) + " %");

    ui->textEditSecondMeasureComments->setPlainText(sourceMeasure->getComments());
    updateSecondCharts(results);
}

void SummaryMeasureDialog::updateMeasureTripleLabelsSummary()
{
    if(!sourceMeasure || !sourceMeasure->hasResults())
        return;

    auto results = sourceMeasure->getResults();

    auto sample = sourceMeasure->getSample();
    ui->valueTripleSampleName->setText(sample->getName());
    ui->valueTripleMaterialName->setText(sample->getMaterialName());
    ui->valueTripleTheoreticalDensity->setText(QString::number(sample->getMaterialDensity()) + " g/cm³");

    Fluid fluid = sourceMeasure->getFluid();
    ui->valueTripleMeasureLiquidName->setText(fluid.getName());
    ui->valueTripleFluidTemp->setText(QString::number(sourceMeasure->getFluidTemperature()) + " °C");
    ui->valueTripleMeasureLiquidDensity->setText(QString::number(sourceMeasure->getFluidDensity(), 'f', 5) + " g/cm³");

    ui->valueTripleMeasureSaturationMethod->setText(utils::getSaturationMethodName(sourceMeasure->getSaturationMethod()));
    ui->valueSaturationBeginDate->setText(sourceMeasure->getSaturationBeginDate().toString("dd-MM-yyyy hh:mm:ss"));
    ui->valueTripleMeasureSaturationTime->setText(QString::number(sourceMeasure->getSaturationTime()) + " min");

    ui->valueTripleMeasureType->setText(sourceMeasure->getType() == MeasurementType::TwoStage ? "Dwustopniowy" : "Trzystopniowy");
    ui->valueTripleMeasureOperator->setText(sourceMeasure->getAuthor());
    QString dateTime = sourceMeasure->getDate().toString("dd-MM-yyyy HH:mm");
    ui->valueTripleMeasureDateTime->setText(dateTime);
    QString endDateTime = sourceMeasure->getEndDate().toString("dd-MM-yyyy HH:mm");
    ui->valueTripleMeasureEndDateTime->setText(endDateTime);

    double dryMass = sourceMeasure->getSampleDryMass();
    ui->valueTripleMeasureDryMass->setText(QString::number(dryMass) + " g");
    double massInFluid = sourceMeasure->getSampleInFluidMass();
    ui->valueTripleMeasureWetMass->setText(QString::number(massInFluid) + " g");
    double saturatedMass = sourceMeasure->getSampleSaturatedMass();
    ui->valueTripleMeasureSaturatedMass->setText(QString::number(saturatedMass) + " g");

    double apparentDensity = results.getApparentDensity();
    ui->valueTripleMeasureApparentDensity->setText(QString::number(apparentDensity) + " g/cm³");
    double relativeDensity = results.getRelativeDensity();
    ui->valueTripleMeasureRelativeDensity->setText(QString::number(relativeDensity) + " %");
    double apparentVolume = results.getApparentVolume();
    ui->valueTripleMeasureApparentVolume->setText(QString::number(apparentVolume) + " cm³");
    double openPoresVolume = results.getOpenPoresVolume();
    ui->valueTripleMeasureOpenPoresVolume->setText(QString::number(openPoresVolume) + " cm³");
    double totalPorosity = results.getTotalPorosity();
    ui->valueTripleMeasureTotalPorosity->setText(QString::number(totalPorosity) + " %");
    double openPorosity = results.getOpenPorosity();
    ui->valueTripleMeasureOpenPorosity->setText(QString::number(openPorosity) + " %");
    double closedPorosity = results.getClosedPorosity();
    ui->valueTripleMeasureClosedPorosity->setText(QString::number(closedPorosity) + " %");
    double waterAbsorption = results.getWaterAbsorption();
    ui->valueTripleMeasureWaterAbsorbability->setText(QString::number(waterAbsorption) + " %");

    ui->textEditTripleMeasureComments->setPlainText(sourceMeasure->getComments());
    updateTripleCharts(results);
}

void SummaryMeasureDialog::setupSecondCharts()
{
    if(ui->frameDensityChartSecond)
    {
        denistyChartSecond = new DensityChartWidget(this);
        denistyChartSecond->setChartType(BaseChartType::BarChart);

        auto layoutDensitySecond = new QVBoxLayout(ui->frameDensityChartSecond);
        layoutDensitySecond->setContentsMargins(0, 0, 0, 0);
        layoutDensitySecond->addWidget(denistyChartSecond);
    }
}

void SummaryMeasureDialog::setupTripleCharts()
{
    if(ui->framePorosityChart)
    {
        porosityChart = new PorosityChartWidget(this);
        porosityChart->setChartType(BaseChartType::PieChart);

        auto layout = new QVBoxLayout(ui->framePorosityChart);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(porosityChart);
    }
    if(ui->frameDensityChart)
    {
        denistyChart = new DensityChartWidget(this);
        denistyChart->setChartType(BaseChartType::BarChart);

        auto layoutDensity = new QVBoxLayout(ui->frameDensityChart);
        layoutDensity->setContentsMargins(0, 0, 0, 0);
        layoutDensity->addWidget(denistyChart);
    }
}

void SummaryMeasureDialog::updateSecondCharts(const MeasurementResults &results)
{
    if(!denistyChartSecond)
        return;
    denistyChartSecond->clearChart();
    denistyChartSecond->updateChart(results);
}

void SummaryMeasureDialog::updateTripleCharts(const MeasurementResults &results)
{
    if(!porosityChart)
        return;
    porosityChart->clearChart();
    porosityChart->updateChart(results);

    if(!denistyChart)
        return;
    denistyChart->clearChart();
    denistyChart->updateChart(results);
}

void SummaryMeasureDialog::onExcelExportButtonClicked()
{
    emit exportMeasuresExcel(QList<const Measurement*>{sourceMeasure.get()});
}
