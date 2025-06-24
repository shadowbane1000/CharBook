#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "module.h"

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

extern class BatteryMonitor BatteryMonitor;

#endif // __BATTERY_MONITOR_H__
