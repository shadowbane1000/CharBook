#include "Widgets/BatteryWidget.h"
#include "batterymonitor.h"
#include "artwork/artwork.h"
#include "display.h"
#include <ArduinoJson.h> // Assuming JsonObject is from ArduinoJson

BatteryWidget::BatteryWidget() : Widget() {
  SetX(0); 
  SetY(0); 
  SetWidth(80); 
  SetHeight(80);
  lastBars = 0;
}

BatteryWidget::BatteryWidget(JsonObject config) : Widget(config) {
  // TODO: Initialize BatteryWidget specific members from config if any
  lastBars = 0;
}

void BatteryWidget::Update()
{
  if(lastBars != BatteryMonitor.Bars){
    Render(0, 0);
  }
}

void BatteryWidget::Render(int16_t parentx, int16_t parenty)
{
  SetX(Display.Width - 80 + parentx);
  SetY(parenty);
  lastBars = BatteryMonitor.Bars;
  switch(lastBars){
  case 0:
    Display.DrawPng(BATTERY0_png, BATTERY0_png_len, GetX(), GetY());
    break;
  case 1:
    Display.DrawPng(BATTERY1_png, BATTERY1_png_len, GetX(), GetY());
    break;
  case 2:
    Display.DrawPng(BATTERY2_png, BATTERY2_png_len, GetX(), GetY());
    break;
  case 3:
    Display.DrawPng(BATTERY3_png, BATTERY3_png_len, GetX(), GetY());
    break;
  case 4:
    Display.DrawPng(BATTERY4_png, BATTERY4_png_len, GetX(), GetY());
    break;
  case 5:
    Display.DrawPng(BATTERY5_png, BATTERY5_png_len, GetX(), GetY());
    break;
  case 6:
    Display.DrawPng(BATTERY6_png, BATTERY6_png_len, GetX(), GetY());
    break;
  }
}
