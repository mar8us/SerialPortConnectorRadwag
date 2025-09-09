#ifndef DENSITY_CHART_H
#define DENSITY_CHART_H

#include "base_chart_widget.h"

enum class DensityChartMode
{
    CompareDensity,
    EfficientyMaterial
};

class DensityChartWidget : public BaseChartWidget
{
    Q_OBJECT

public:
    explicit DensityChartWidget(QWidget *parent = nullptr);
    ~DensityChartWidget() override;

protected:
    QString getCategoryName() const override;
    QString getTitle() const override;
    QList<ChartDataPoint> extractChartData(const MeasurementResults& results) override;
    void setChartTypeCombo() override;

    DensityChartMode getSelectedMode() const;

private:
    QList<ChartDataPoint> createDensityComparisonData(const MeasurementResults& results);
    QList<ChartDataPoint> createMaterialEfficiencyData(const MeasurementResults& results);
    // QList<ChartDataPoint> createDetailedDensityData(const MeasurementResults& results);

    QColor getTheoreticalDensityColor() const;
    QColor getApparentDensityColor() const;
    QColor getRelativeDensityColor() const;
    QColor getUtilizedPotentialColor() const;
    QColor getWastedPotentialColor() const;
};

#endif // DENSITY_CHART_H
