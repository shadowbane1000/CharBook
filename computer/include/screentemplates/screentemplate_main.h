#include <Arduino.h>
#include <strings.h>

#include "screentemplate.h"
#include "widgets/cardwidget.h"
#include "widgets/settingswidget.h"
#include "widgets/wirelesswidget.h"
#include "widgets/batterywidget.h"

class ScreenTemplate_Main : public ScreenTemplate
{
  WirelessWidget wirelessButton;
  BatteryWidget batteryWidget;
  SettingsWidget settingsWidget;

public:
  ScreenTemplate_Main(){
    Children.push_back(&wirelessButton);
    Children.push_back(&batteryWidget);
    Children.push_back(&settingsWidget);
  }
  virtual const char* Name(){ return "Main"; }
};
