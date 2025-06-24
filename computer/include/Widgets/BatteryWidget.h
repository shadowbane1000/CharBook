#ifndef __BATTERY_WIDGET_H__
#define __BATTERY_WIDGET_H__

#include "Widget.h"
#include <ArduinoJson.h>

class BatteryWidget : public Widget
{
  uint8_t lastBars;
public:
  BatteryWidget();
  BatteryWidget(JsonObject config);

  virtual void Update();
  virtual void Render(int16_t parentx, int16_t parenty);
};

#endif // __BATTERY_WIDGET_H__
