#include <Arduino.h>

#include "widgets/settingswidget.h"
#include "display.h"
#include "artwork/artwork.h"
#include "screen.h"

void SettingsWidget::HandleTouch(uint16_t x, uint16_t y)
{
  Serial.println("Settings touched");
  Screen.Push("SETTINGS");
}

void SettingsWidget::Update()
{
}

void SettingsWidget::Render()
{
  x = Display.Width - 80*2;
  y = 0;
  Display.DrawPng(SETTING_png, SETTING_png_len, x, y);
}

