#ifndef __BATTERY_WIDGET_H__
#define __BATTERY_WIDGET_H__

#include "fixedsizewidget.h"

class BatteryWidget : public FixedSizeWidget
{
  uint8_t lastBars;
public:
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
};

#endif // __BATTERY_WIDGET_H__
