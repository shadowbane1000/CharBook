#include "widgets/batterywidget.h"
#include "batterymonitor.h"
#include "artwork/artwork.h"
#include "display.h"

void BatteryWidget::Update()
{
  if(lastBars != BatteryMonitor.Bars){
    Render();
  }
}

void BatteryWidget::Render()
{
  x = Display.Width - 80;
  y = 0;
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
