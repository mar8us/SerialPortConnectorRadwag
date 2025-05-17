#ifndef MEASUREMENT_SORT_FILTER_PROXY_MODEL_H
#define MEASUREMENT_SORT_FILTER_PROXY_MODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class MeasurementSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MeasurementSortFilterProxyModel(QObject* parent = nullptr);

    void setFilterColumn(int column);
    void setFilter(const QString& text);

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString filterString;
};

#endif // MEASUREMENT_SORT_FILTER_PROXY_MODEL_H
