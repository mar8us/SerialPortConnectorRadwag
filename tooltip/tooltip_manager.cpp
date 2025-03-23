#include "tooltip_manager.h"
#include <QFile>
#include <QBuffer>
#include <QToolTip>
#include <QEvent>
#include <QDebug>
#include <QRegularExpression>

TooltipManager::TooltipManager() : QObject(nullptr)
{
    globalStyleSheet = "QToolTip { "
                         "background-color: #F5F5F5; "
                         "color: #333333; "
                         "border: 1px solid #CCCCCC; "
                         "border-radius: 4px; "
                         "padding: 6px; "
                         "font-size: 12px; "
                         "}";
    QToolTip::setFont(QFont("Arial", 10));
    qApp->setStyleSheet(qApp->styleSheet() + globalStyleSheet);
}

TooltipManager::~TooltipManager()
{

}

TooltipManager& TooltipManager::getInstance()
{
    static TooltipManager instance;
    return instance;
}

void TooltipManager::setGlobalStyle(const QString& styleSheet)
{
    globalStyleSheet = styleSheet;

    QString appStyleSheet = qApp->styleSheet();
    appStyleSheet.remove(QRegularExpression("QToolTip\\s*\\{[^\\}]*\\}"));
    appStyleSheet += globalStyleSheet;
    qApp->setStyleSheet(appStyleSheet);

    // Odśwież tooltipów dla wszystkich zarejestrowanych widgetów
    for(auto it = tooltips.begin(); it != tooltips.end(); it++)
        applyTooltip(it.key());
}

bool TooltipManager::registerImage(const QString& key, const QString& imagePath, int width, int height)
{
    QPixmap pixmap(imagePath);
    if(pixmap.isNull())
    {
        qWarning() << "Nie można załadować obrazu:" << imagePath;
        return false;
    }

    if(width > 0 && height > 0)
    {
        pixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if(pixmap.isNull())
    {
        qWarning() << "Nie można załadować obrazu:" << imagePath;
        return false;
    }
    images[key] = pixmap;

    return true;
}

void TooltipManager::registerTooltip(QWidget* widget, const QString& title, const QString& text, const QString& imageKey, TooltipMode mode)
{
    if(!widget)
        return;

    TooltipData data;
    data.title = title;
    data.text = text;
    data.imageKey = imageKey;
    data.mode = mode;

    tooltips[widget] = data;
    applyTooltip(widget);
}

void TooltipManager::updateTooltipText(QWidget* widget, const QString& text)
{
    if(!widget || !tooltips.contains(widget))
        return;

    tooltips[widget].text = text;
    applyTooltip(widget);
}

void TooltipManager::updateTooltipTitle(QWidget* widget, const QString& title)
{
    if(!widget || !tooltips.contains(widget))
        return;

    tooltips[widget].title = title;
    applyTooltip(widget);
}

void TooltipManager::updateTooltipImage(QWidget* widget, const QString& imageKey)
{
    if(!widget || !tooltips.contains(widget))
        return;

    tooltips[widget].imageKey = imageKey;
    applyTooltip(widget);
}

void TooltipManager::updateTooltipMode(QWidget* widget, TooltipMode mode)
{
    if(!widget || !tooltips.contains(widget))
        return;

    tooltips[widget].mode = mode;
    applyTooltip(widget);
}

QString TooltipManager::getImageHtml(const QString& imageKey)
{
    if(imageKey.isEmpty() || !images.contains(imageKey))
        return QString();

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    images[imageKey].save(&buffer, "PNG");
    return QString("<img src='data:image/png;base64,%1' />").arg(QString(byteArray.toBase64()));
}

QString TooltipManager::createTooltipHtml(const QString& title, const QString& text, const QString& imageKey, TooltipMode mode)
{
    QString imageHtml = getImageHtml(imageKey);

    QString titleHtml = title.isEmpty() ? "" : QString("<div style='font-size: 14px; font-weight: bold; margin-bottom: 6px;'>%1</div>").arg(title);
    QString textHtml = text.isEmpty() ? "" : QString("<div style='font-size: 12px;'>%1</div>").arg(text);

    QString contentHtml = titleHtml + textHtml;


    if(imageHtml.isEmpty())
        return contentHtml;
    if(contentHtml.isEmpty())
        return imageHtml;

    switch (mode)
    {
        case IMAGE_ONLY:
            return imageHtml;
        case TEXT_ONLY:
            return contentHtml;
        case IMAGE_TOP:
            return QString("<div style='text-align:center;'>%1</div>%2").arg(imageHtml, contentHtml);
        case IMAGE_BOTTOM:
            return QString("%1<div style='text-align:center; margin-top: 8px;'>%2</div>").arg(contentHtml, imageHtml);
        case IMAGE_LEFT:
            return QString("<table><tr><td style='vertical-align:middle;'>%1</td>"
                           "<td style='vertical-align:middle; padding-left:10px;'>%2</td></tr></table>")
                .arg(imageHtml, contentHtml);
        case IMAGE_RIGHT:
            return QString("<table><tr><td style='vertical-align:middle;'>%1</td>"
                           "<td style='vertical-align:middle; padding-right:10px;'>%2</td></tr></table>")
                .arg(contentHtml, imageHtml);
        default:
            return QString("%1<br>%2").arg(imageHtml, contentHtml);
    }
}

void TooltipManager::applyTooltip(QWidget* widget)
{
    if(!widget || !tooltips.contains(widget))
        return;

    const TooltipData& data = tooltips[widget];
    QString tooltipHtml = createTooltipHtml(data.title, data.text, data.imageKey, data.mode);

    widget->setToolTip(tooltipHtml);
}
