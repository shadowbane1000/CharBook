#ifndef __SETTINGS_WIDGET_H__
#define __SETTINGS_WIDGET_H__

#include "Widget.h"
#include <ArduinoJson.h> // Added for JsonObject

class SettingsWidget : public Widget
{
public:
  SettingsWidget() { SetX(0); SetY(0); SetWidth(80); SetHeight(80); }
  SettingsWidget(JsonObject config) : Widget(config) {}
  virtual void Update();
  virtual void Render(int16_t parentx, int16_t parenty);
  virtual bool HandleTouch(uint16_t x, uint16_t y);

};

#endif //__SETTINGS_WIDGET_H__
