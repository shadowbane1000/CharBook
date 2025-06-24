#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "src/module.h"
#include "screentemplate.h"

class BatteryMonitor : public Module
{
  int nextSampleMillis;
  void PerformRead();
public:
  void Initialize();
  virtual Module::UpdateResponse Update();

  // 0-4 bars, 5 bars == charging, 6 bars == unknown state
  uint8_t Bars;
};

class BatteryStatusIndicator : public FixedSizeWidget
{
  uint8_t lastBars;
public:
  BatteryStatusIndicator(){
    x = Display.Width - 80;
    y = 0;
  }
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
};

extern BatteryMonitor BatteryMonitor;

#endif // __BATTERY_MONITOR_H__
