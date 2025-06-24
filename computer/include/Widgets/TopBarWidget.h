#ifndef __TOP_BAR_WIDGET_H__
#define __TOP_BAR_WIDGET_H__

#include "ContainerWidget.h"
#include "WirelessWidget.h"
#include "BatteryWidget.h"
#include "SettingsWidget.h"
#include "PopScreenWidget.h"

class TopBarWidget : public ContainerWidget
{
    WirelessWidget wirelessButton;
    BatteryWidget batteryWidget;
    SettingsWidget settingsWidget;
    PopScreenWidget popScreenWidget;
    bool popButtonVisible;

public:
    TopBarWidget();
    TopBarWidget(JsonObject config) : ContainerWidget(config) {}
    
    virtual void Layout() override;

    void ShowPopButton();
    void HidePopButton();
};

#endif // __TOP_BAR_WIDGET_H__
