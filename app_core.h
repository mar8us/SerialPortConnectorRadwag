#ifndef APP_CORE_H
#define APP_CORE_H

#include "radwag/radwag_scale_connector.h"
#include <memory>

class AppCore
{
public:
    AppCore();;
    ~AppCore();

    void connectScale(const QString& port, const std::shared_ptr<const Device>& device);
    void disconnectScale();

    bool hasConnectionWithScale() const;
    const RadwagScaleConnector* getScaleConnector() const;

private:
    std::unique_ptr<RadwagScaleConnector> radwagScaleConnector;
};

extern AppCore appCore;

#endif
