#include "denisty_chart.h"
#include <QDebug>
#include <qwidget.h>

DensityChartWidget::DensityChartWidget(QWidget *parent)
    : BaseChartWidget(parent)
{
    setChartTypeCombo();
    qDebug() << "DensityChartWidget created";
}

DensityChartWidget::~DensityChartWidget()
{
    qDebug() << "DensityChartWidget destroyed";
}

QString DensityChartWidget::getCategoryName() const
{
    return "Gęstość";
}

QString DensityChartWidget::getTitle() const
{
    double relativeDensity = currentResults.getRelativeDensity();
    double apparentDensity = currentResults.getApparentDensity();

    switch(getCurrentChartType())
    {
    case BaseChartType::BarChart:
        return QString("Porównanie typów gęstości (gęstość pozorna: %1)")
            .arg(formatValue(apparentDensity, "g/cm³"));

    case BaseChartType::PieChart:
        return QString("Efektywność wykorzystania materiału (zagęszczenie: %1%)")
            .arg(formatPercentage(relativeDensity));

    default:
        return QString("Analiza gęstości (zagęszczenie: %1%)")
            .arg(formatPercentage(relativeDensity));
    }
}

void DensityChartWidget::setChartTypeCombo()
{
    chartTypeCombo->addItem("Porównanie gęstości", static_cast<int>(DensityChartMode::CompareDensity));
    chartTypeCombo->addItem("Efektywność materiału", static_cast<int>(DensityChartMode::EfficientyMaterial));
    chartTypeCombo->setCurrentIndex(0);
}

DensityChartMode DensityChartWidget::getSelectedMode() const
{
    return static_cast<DensityChartMode>(chartTypeCombo->currentIndex());
}

QList<ChartDataPoint> DensityChartWidget::extractChartData(const MeasurementResults& results)
{
    DensityChartMode mode = getSelectedMode();

    switch(mode)
    {
        case DensityChartMode::CompareDensity:
            currentChartType = BaseChartType::BarChart;
            return createDensityComparisonData(results);

        case DensityChartMode::EfficientyMaterial:
            currentChartType = BaseChartType::PieChart;
            return createMaterialEfficiencyData(results);

        default:
            QList<ChartDataPoint>();
    }
}

QList<ChartDataPoint> DensityChartWidget::createDensityComparisonData(const MeasurementResults& results)
{
    QList<ChartDataPoint> data;

    double theoreticalDensity = results.getTheoreticalDensity();
    double apparentDensity = results.getApparentDensity();
    double fluidDensity = results.getFluidDensity();
    double relativeDensity = results.getRelativeDensity();

    data.append(ChartDataPoint(
        "Gęstość teoretyczna",
        theoreticalDensity,
        getTheoreticalDensityColor(),
        "g/cm³",
        QString("Gęstość teoryczna materiału: %1 g/cm³").arg(formatValue(theoreticalDensity))
        ));

    data.append(ChartDataPoint(
        "Gęstość pozorna",
        apparentDensity,
        getApparentDensityColor(),
        "g/cm³",
        QString("Rzeczywista zmierzona gęstość materiału: %1 g/cm³ (%2% teoretycznej)")
            .arg(formatValue(apparentDensity))
            .arg(formatPercentage(relativeDensity))
        ));

    return data;
}

QList<ChartDataPoint> DensityChartWidget::createMaterialEfficiencyData(const MeasurementResults& results)
{
    QList<ChartDataPoint> data;

    double relativeDensity = results.getRelativeDensity();
    double wastedPotential = 100.0 - relativeDensity;

    data.append(ChartDataPoint(
        QString("Materiał stały (%1%)").arg(formatPercentage(relativeDensity)),
        relativeDensity,
        getUtilizedPotentialColor(),
        "%",
        QString("Zagęszczenie materiału: %1%")
            .arg(formatPercentage(relativeDensity))
        ));

    data.append(ChartDataPoint(
        QString("Pory i pustki (%1%)").arg(formatPercentage(wastedPotential)),
        wastedPotential,
        getWastedPotentialColor(),
        "%",
        QString("Objętość zajęta przez pory: %1%")
            .arg(formatPercentage(wastedPotential))
        ));

    return data;
}

// QList<ChartDataPoint> DensityChartWidget::createDetailedDensityData(const MeasurementResults& results)
// {
//     QList<ChartDataPoint> data;

    // double theoreticalDensity = results.getTheoreticalDensity();
    // double apparentDensity = results.getApparentDensity();
    // double relativeDensity = results.getRelativeDensity();
    // double fluidDensity = results.getFluidDensity();
    // double apparentVolume = results.getApparentVolume();

    // data.append(ChartDataPoint(
    //     "Gęstość teoretyczna",
    //     theoreticalDensity,
    //     getTheoreticalDensityColor(),
    //     "g/cm³",
    //     QString("Referencyjna gęstość materiału bez porów: %1 g/cm³").arg(formatValue(theoreticalDensity))
    //     ));

    // data.append(ChartDataPoint(
    //     "Gęstość pozorna",
    //     apparentDensity,
    //     getApparentDensityColor(),
    //     "g/cm³",
    //     QString("Gęstość materiału z porami: %1 g/cm³").arg(formatValue(apparentDensity))
    //     ));

    // data.append(ChartDataPoint(
    //     "Gęstość względna",
    //     relativeDensity,
    //     getRelativeDensityColor(),
    //     "%",
    //     QString("Stopień zagęszczenia materiału: %1%").arg(formatPercentage(relativeDensity))
    //     ));

    // data.append(ChartDataPoint(
    //     "Objętość pozorna",
    //     apparentVolume,
    //     QColor("#9B59B6"), // Fioletowy
    //     "cm³",
    //     QString("Całkowita objętość próbki z porami: %1 cm³").arg(formatValue(apparentVolume))
    //     ));

    // qDebug() << "Detailed density data created:" << data.size() << "points";
//     return data;
// }

QColor DensityChartWidget::getTheoreticalDensityColor() const
{
    return QColor("#2C3E50"); // Ciemny niebieski - teoretyczna, referencyjna
}

QColor DensityChartWidget::getApparentDensityColor() const
{
    return QColor("#27AE60"); // Zielony - rzeczywista, zmierzona
}

// QColor DensityChartWidget::getRelativeDensityColor() const
// {
//     return QColor("#E67E22"); // Pomarańczowy - wskaźnik procentowy
// }

QColor DensityChartWidget::getUtilizedPotentialColor() const
{
    return QColor("#27AE60"); // Zielony
}

QColor DensityChartWidget::getWastedPotentialColor() const
{
    return QColor("#E74C3C"); // Czerwony
}
