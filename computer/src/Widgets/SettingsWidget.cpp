#include <Arduino.h>

#include "Widgets/SettingsWidget.h"
#include "display.h"
#include "artwork/artwork.h"
#include "screen.h"

bool SettingsWidget::HandleTouch(uint16_t x, uint16_t y)
{
  Serial.println("Settings touched");
  Screen.Push("SETTINGS");
  return true;
}

void SettingsWidget::Update()
{
}

void SettingsWidget::Render(int16_t parentx, int16_t parenty)
{
  SetX(Display.Width - 80*2);
  SetY(0);
  Display.DrawPng(SETTING_png, SETTING_png_len, GetX()+parentx, GetY()+parenty);
}

