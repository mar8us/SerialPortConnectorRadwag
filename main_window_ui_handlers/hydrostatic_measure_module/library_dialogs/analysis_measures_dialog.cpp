#include "analysis_measures_dialog.h"
#include "ui_analysis_measures_dialog.h"
#include "../../../radwag/measure_statistic_analyzer.h"
#include <qdialog.h>
#include <qscreen.h>
#include <qstandardpaths.h>
#include <QFileDialog>
#include <QMessageBox>
// #include "../../../utils.h"
 #include "../../../config.h"


MeasurementTableModel::MeasurementTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , sampleCount(0)
{

}

void MeasurementTableModel::setAnalysisResults(const AnalysisResult& results)
{
    beginResetModel();

    nodes.clear();
    sampleCount = results.getMeasuresCount();

    nodes.push_back(std::make_unique<ResultNode>(&results.getDryMassResult(), "Masa sucha [g]"));
    nodes.push_back(std::make_unique<ResultNode>(&results.getWetMassResult(), "Masa w cieczy [g]"));
    nodes.push_back(std::make_unique<ResultNode>(&results.getSaturatedMassResult(), "Masa nasycona [g]"));
    nodes.push_back(std::make_unique<ResultNode>(&results.getDensityResult(), "Gęstość pozorna [g/cm³]"));
    nodes.push_back(std::make_unique<ResultNode>(&results.getRelativeDensityResult(), "Gęstość względna [%]"));
    nodes.push_back(std::make_unique<ResultNode>(&results.getPorosityResult(), "Porowatość całkowita [%]"));

    endResetModel();
}

void MeasurementTableModel::updateResults()
{
#ifdef TABLE_FULL_STAT
    QModelIndex topLeft = index(0, getStatColumnIndex(STAT_MEAN_OFFSET));
    QModelIndex bottomRight = index(static_cast<int>(nodes.size()) - 1, getStatColumnIndex(STAT_CV_OFFSET));
    emit dataChanged(topLeft, bottomRight);
#else
    QModelIndex topLeft = index(0, COL_FIRST_SAMPLE);
    QModelIndex bottomRight = index(static_cast<int>(nodes.size()) - 1, COL_FIRST_SAMPLE + sampleCount - 1);
    emit dataChanged(topLeft, bottomRight);
#endif
}

void MeasurementTableModel::clear()
{
    beginResetModel();
    nodes.clear();
    sampleCount = 0;
    endResetModel();
}

int MeasurementTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(nodes.size());
}

int MeasurementTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    if(sampleCount == 0)
        return 0;

#ifdef TABLE_FULL_STAT
    return TableColumns::COL_FIRST_SAMPLE + sampleCount + StatisticTableColmuns::STAT_COL_COUNT;
#else
    return TableColumns::COL_FIRST_SAMPLE + sampleCount;
#endif
}

QVariant MeasurementTableModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() >= static_cast<int>(nodes.size()))
        return QVariant();

    const ResultNode* node = nodes[index.row()].get();
    const BaseAnalysisResult* result = node->analysisResult;
    const AnalysisType type = result->getAnalysisType();

    if(!result)
        return QVariant();

    switch(role)
    {
        case Qt::DisplayRole:
            return getDisplayData(index, *result);

        case Qt::UserRole:
            return QVariant::fromValue(const_cast<BaseAnalysisResult*>(result));

        case Qt::BackgroundRole:
        {
            int col = (index.column());


            if(type == AnalysisType::Density || type == AnalysisType::Porosity)
            {
                if(col == getStatColumnIndex(STAT_MEAN_OFFSET))
                    return QBrush(QColor("#1F4CB3"));
                else if (col == getStatColumnIndex(STAT_STDDEV_OFFSET))
                    return QBrush(QColor("#20439A"));
                else if (col == getStatColumnIndex(STAT_CV_OFFSET))
                    return QBrush(QColor("#203A83"));
                else if (col == getStatColumnIndex(STAT_UNCERTAINTY_OFFSET))
                    return QBrush(QColor("#1F316C"));
                else if (col == getStatColumnIndex(STAT_SUMMARY_OFFSET))
                    return QBrush(QColor("#1C2856"));
                else
                    return QBrush(QColor("#1F316C"));//#1B56CC
            }
            return QVariant();
        }

        case Qt::TextAlignmentRole:
            if(index.column() == COL_PARAMETER)
                return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
            return QVariant::fromValue(Qt::AlignRight | Qt::AlignHCenter);

        case Qt::FontRole:
        {
            int col = index.column();
            if(col == COL_PARAMETER)
            {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }

            if(type == AnalysisType::Mass)
                return QVariant();

            // if(col == COL_PARAMETER ||
            //     col == getStatColumnIndex(STAT_MEAN_OFFSET) ||
            //     col == getStatColumnIndex(STAT_STDDEV_OFFSET) ||
            //     col == getStatColumnIndex(STAT_CV_OFFSET) ||
            //     col == getStatColumnIndex(STAT_UNCERTAINTY_OFFSET) ||
            //     col == getStatColumnIndex(STAT_SUMMARY_OFFSET))
            // {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            // }

            // return QVariant();
        }

        default:
            return QVariant();
    }
}

QVariant MeasurementTableModel::getDisplayData(const QModelIndex& index, const BaseAnalysisResult& result) const
{
    int col = index.column();

    if(col == COL_PARAMETER)
    {
        return nodes[index.row()]->parameterName;
    }
    else if (col >= COL_FIRST_SAMPLE && col < COL_FIRST_SAMPLE + sampleCount)
    {
        int sampleIndex = col - COL_FIRST_SAMPLE;
        auto values = result.getIndividualValues();
        if(sampleIndex < static_cast<int>(values.size()))
        {
            int precision = getPrecisionForValues(result);
            return formatValue(values[sampleIndex], precision);
        }
    }
    else
    {
        #ifdef TABLE_FULL_STAT
        int statOffset = col - (COL_FIRST_SAMPLE + sampleCount);
        switch(statOffset)
        {
            case STAT_MEAN_OFFSET:
                return formatValue(result.getMean(), 4);
            case STAT_STDDEV_OFFSET:
                return formatValue(result.getStandardDeviation(), 4);
            case STAT_CV_OFFSET:
                return QString("%1 %2").arg(formatValue(result.getVariationCoefficient(), 2), "%");
            case STAT_UNCERTAINTY_OFFSET:
                return QString("±%1 %2").arg(result.getUncertainty(), 0, 'f', 4).arg(result.getUnitSymbol());
            case STAT_SUMMARY_OFFSET:
                return result.getFinalResult();
        }
        #endif
    }

    return QVariant();
}

QVariant MeasurementTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal)
    {
        if(section == COL_PARAMETER)
            return "Parametr";

        else if(section >= COL_FIRST_SAMPLE && section < COL_FIRST_SAMPLE + sampleCount)
        {
            int sampleIndex = section - COL_FIRST_SAMPLE;

            return QString("Pomiar %1 (%2)")
                .arg(sampleIndex + 1)
                .arg(nodes[0].get()->analysisResult->getMeasurements()[sampleIndex]->getEndDate().toString("dd-MM-yyyy hh:mm"));
        }
        else
        {
            #ifdef TABLE_FULL_STAT
            int statOffset = section - (COL_FIRST_SAMPLE + sampleCount);
            switch(statOffset)
            {
                case STAT_MEAN_OFFSET: return "Średnia";
                case STAT_STDDEV_OFFSET: return "Odch. std.";
                case STAT_CV_OFFSET: return "Współ. zmienności";

                case STAT_UNCERTAINTY_OFFSET: return "Niepewność pomiarowa";
                case STAT_SUMMARY_OFFSET: return "Podsumowanie";
            }
            #endif
        }
    }
    else if(orientation == Qt::Vertical)
        return QString::number(section + 1);

    return QVariant();
}

int MeasurementTableModel::getSampleColumnIndex(int sampleIndex) const
{
    return COL_FIRST_SAMPLE + sampleIndex;
}

int MeasurementTableModel::getStatColumnIndex(int statOffset) const
{
#ifdef TABLE_FULL_STAT
    return COL_FIRST_SAMPLE + sampleCount + statOffset;
#else
    return -1;
#endif
}

QString MeasurementTableModel::formatValue(double value, int precision) const
{
    return QString::number(value, 'f', precision);
}

int MeasurementTableModel::getPrecisionForValues(const BaseAnalysisResult& result) const
{
    if(result.getUnitSymbol() == "%")
        return 2;
    return 4;
}



AnalysisMeasuresDialog::AnalysisMeasuresDialog(const std::vector<std::shared_ptr<const Measurement>>& measurements, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AnalysisMeasuresDialog)
    , results(new AnalysisResult(measurements))
    // , densityResult(results.getDensityResult())
    // , porosityResult(results.getPorosityResult())
    , measurements(results->getMeasurements())
{
    ui->setupUi(this);
    setWindowTitle("Analiza statystyczna wybranych pomiarów");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initialize();
    updateView();

    #ifdef TABLE_FULL_STAT
    ui->groupBoxDensityResults->setVisible(false);
    ui->groupBoxPorosityResults->setVisible(false);
    #endif
}

AnalysisMeasuresDialog::~AnalysisMeasuresDialog()
{
    delete results;
    delete ui;
}

void AnalysisMeasuresDialog::updateView()
{
    const auto &densityResult = results->getDensityResult();
    ui->editMeasuresSeries->setText(densityResult.getSeriesName());
    ui->editSamplesMaterial->setText(densityResult.getMaterialName());
    ui->editMaterialDenisty->setText(QString::number(densityResult.getTheoreticalDensity()) + "g/cm3");
    ui->lineEditDensityVariationCoeff->setText(QString("%1%").arg(densityResult.getVariationCoefficient(), 0, 'f', 2));

    ui->lineEditDensityMean->setText(QString("%1 g/cm³").arg(densityResult.getMean(), 0, 'f', 3));
    ui->lineEditDensityStdDev->setText(QString("%1 g/cm³").arg(densityResult.getStandardDeviation(), 0, 'f', 4));
    ui->lineEditDensityUncertainty->setText(QString("±%1 g/cm³").arg(densityResult.getUncertainty(), 0, 'f', 3));
    ui->lineEditDensityFinalResult->setText(densityResult.getFinalResult());

    const auto &porosityResult = results->getPorosityResult();
    ui->lineEditPorosityMean->setText(QString("%1 %").arg(porosityResult.getMean(), 0, 'f', 2));
    ui->lineEditPorosityStdDev->setText(QString("%1 %").arg(porosityResult.getStandardDeviation(), 0, 'f', 3));
    ui->lineEditPorosityVariationCoeff->setText(QString("%1%").arg(porosityResult.getVariationCoefficient(), 0, 'f', 2));
    ui->lineEditPorosityUncertainty->setText(QString("±%1 %").arg(porosityResult.getUncertainty(), 0, 'f', 2));
    ui->lineEditPorosityFinalResult->setText(porosityResult.getFinalResult());
}

const AnalysisResult &AnalysisMeasuresDialog::getAnalysisResult() const
{
    return *results;
}

void AnalysisMeasuresDialog::setupTableView()
{
    tableModel = new MeasurementTableModel(this);
    ui->tableView->setModel(tableModel);

    tableModel->setAnalysisResults(*results);

    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView->resizeColumnsToContents();

#ifdef TABLE_FULL_STAT
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
#endif
}

void AnalysisMeasuresDialog::onConfidenceLevelChanged(int index)
{
    double newConfidence = ui->comboBoxConfidence->itemData(index).toDouble();
    results->setConfidenceLevel(newConfidence);
    tableModel->updateResults();

    ui->lineEditSampleCount->setText(QString::number(results->getMeasuresCount()));
    updateView();
}

void AnalysisMeasuresDialog::onExportExcelButtonClicked()
{
    emit exportAnalysisExcel(results);
}

void AnalysisMeasuresDialog::initialize()
{
    setupTableView();
    ui->lineEditSampleCount->setText(QString::number(results->getMeasuresCount()));
    fillConfidenceComboBox();
    connectSignals();
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
    connect(ui->buttonClose, &QPushButton::clicked, this, &QMainWindow::close);
    connect(ui->buttonExportExcel, &QPushButton::clicked, this, &AnalysisMeasuresDialog::onExportExcelButtonClicked);
    connect(ui->comboBoxConfidence, &QComboBox::currentIndexChanged, this, &AnalysisMeasuresDialog::onConfidenceLevelChanged);
}
