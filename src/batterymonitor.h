#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "service.h"

class BatteryMonitor : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
};

extern BatteryMonitor BatteryMonitor;

#endif // __BATTERY_MONITOR_H__
