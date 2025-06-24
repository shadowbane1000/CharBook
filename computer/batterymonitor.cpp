#include <Arduino.h>
#include "batterymonitor.h"

#include "src/module.h"
#include "src/artwork/artwork.h"
#include "wireless.h"

#define VAL_AT_0 (2048/5)
#define VAL_AT_1 (VAL_AT_0 * 2)
#define VAL_AT_2 (VAL_AT_0 * 3)
#define VAL_AT_3 (VAL_AT_0 * 4)
#define VAL_AT_4 (VAL_AT_0 * 5)

#define BAT_ADC 14
#define SAMPLE_FREQ (10 * 60 * 1000) // 10 minutes

void BatteryMonitor::Initialize()
{
  analogReadResolution(12);
  PerformRead();
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
    Serial.print("Battery ADC value: ");
    Serial.print(val);
    Serial.println();
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

  PerformRead();

  return SLEEP_OKAY;
}

void BatteryStatusIndicator::Update()
{
  if(lastBars != BatteryMonitor.Bars){
    Render();
  }
}

void BatteryStatusIndicator::Render()
{
  lastBars = BatteryMonitor.Bars;
  switch(lastBars){
  case 0:
    Display.DrawPng(BATTERY0_png, BATTERY0_png_len, x, y);
    break;
  case 1:
    Display.DrawPng(BATTERY1_png, BATTERY1_png_len, x, y);
    break;
  case 2:
    Display.DrawPng(BATTERY2_png, BATTERY2_png_len, x, y);
    break;
  case 3:
    Display.DrawPng(BATTERY3_png, BATTERY3_png_len, x, y);
    break;
  case 4:
    Display.DrawPng(BATTERY4_png, BATTERY4_png_len, x, y);
    break;
  case 5:
    Display.DrawPng(BATTERY5_png, BATTERY5_png_len, x, y);
    break;
  case 6:
    Display.DrawPng(BATTERY6_png, BATTERY6_png_len, x, y);
    break;
  }
}
