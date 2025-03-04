#include "batterymonitor.h"

#include "service.h"

#define BAT_ADC 14

void BatteryMonitor::Initialize()
{

}

Module::UpdateResponse BatteryMonitor::Update()
{
  return SLEEP_OKAY;
}
