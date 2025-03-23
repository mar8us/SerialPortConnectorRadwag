#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QApplication>
#include <QToolTip>
#include <QStyle>


class TooltipManager : public QObject
{
    Q_OBJECT

public:
    enum TooltipMode
    {
        TEXT_ONLY,
        IMAGE_ONLY,
        IMAGE_TOP,
        IMAGE_LEFT,
        IMAGE_RIGHT,
        IMAGE_BOTTOM
    };

    static TooltipManager& getInstance();

    void setGlobalStyle(const QString& styleSheet);
    void registerTooltip(QWidget* widget, const QString& title, const QString& text, const QString& imageKey = QString(), TooltipMode mode = TEXT_ONLY);
    bool registerImage(const QString& key, const QString& imagePath, int width = -1, int height = -1);


    void updateTooltipText(QWidget* widget, const QString& text);
    void updateTooltipTitle(QWidget* widget, const QString& title);
    void updateTooltipImage(QWidget* widget, const QString& imageKey);
    void updateTooltipMode(QWidget* widget, TooltipMode mode);

private:
    TooltipManager();
    ~TooltipManager();

    QString createTooltipHtml(const QString& title, const QString& text, const QString& imageKey, TooltipMode mode);

    struct TooltipData
    {
        QString title;
        QString text;
        QString imageKey;
        TooltipMode mode;
    };

    QMap<QString, QPixmap> images;
    QMap<QWidget*, TooltipData> tooltips;
    QString globalStyleSheet;

    void applyTooltip(QWidget* widget);
    QString getImageHtml(const QString& imageKey);
};

#endif // TOOLTIPMANAGER_H
