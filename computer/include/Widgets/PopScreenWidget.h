#ifndef __POP_SCREEN_WIDGET_H__
#define __POP_SCREEN_WIDGET_H__

#include "Widget.h"
#include <ArduinoJson.h> // Added for JsonObject

class PopScreenWidget : public Widget
{
public:
  PopScreenWidget() { SetX(80); SetY(0); SetWidth(80); SetHeight(80); }
  PopScreenWidget(JsonObject config) : Widget(config) {}

  virtual void Update();
  virtual void Render(int16_t parentx, int16_t parenty);
  virtual bool HandleTouch(uint16_t x, uint16_t y);

};

#endif // __POP_SCREEN_WIDGET_H__
