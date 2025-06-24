#include <Arduino.h>
#include "batterymonitor.h"

#include "module.h"
#include "artwork/artwork.h"
#include "wireless.h"
#include "display.h"

#define VAL_AT_0 (2048/5)
#define VAL_AT_1 (VAL_AT_0 * 2)
#define VAL_AT_2 (VAL_AT_0 * 3)
#define VAL_AT_3 (VAL_AT_0 * 4)
#define VAL_AT_4 (VAL_AT_0 * 5)

#define BAT_ADC 14
#define SAMPLE_FREQ (10 * 60 * 1000) // 10 minutes

void BatteryMonitor::Initialize()
{
//  analogReadResolution(12);
//  PerformRead();
}

void BatteryMonitor::PerformRead()
{
  // if wifi is on, troubles might occur.  Try reading this:
  // https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/adc.html
  if(Wireless.State != Wireless::WIFI_AVAILABLE && Wireless.State != Wireless::WIFI_UNAVAILABLE)
  {
    // if the Wireless radio is on, we can't read..., but we will read immediately if the wifi gets turned off
    // and our timer has expired.
    Bars = 6;
  }
  else
  {
    int val = analogRead(BAT_ADC);
    Serial.printf("Battery ADC value: %d\r\n", val);
    if(val < VAL_AT_0) Bars = 0;
    else if(val < VAL_AT_1) Bars = 1;
    else if(val < VAL_AT_2) Bars = 2;
    else if(val < VAL_AT_3) Bars = 3;
    else if(val < VAL_AT_4) Bars = 4;
    else Bars = 4;
    nextSampleMillis = millis() + SAMPLE_FREQ;
  }
}

Module::UpdateResponse BatteryMonitor::Update()
{
  if(millis() < nextSampleMillis) return SLEEP_OKAY;

//  PerformRead();

  return SLEEP_OKAY;
}

