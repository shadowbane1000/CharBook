#ifndef __SCREEN_TEMPLATE_STANDARD_H__
#define __SCREEN_TEMPLATE_STANDARD_H__

#include <Arduino.h>
#include <strings.h>

#include <ArduinoJson.h>

#include "ScreenTemplate.h"
#include "Widgets/TileHolderWidget.h"
#include "Widgets/TopBarWidget.h"

class ScreenTemplate_Standard : public ScreenTemplate
{
  TopBarWidget topBarWidget;
  TileHolderWidget* tileHolder;

public:
  ScreenTemplate_Standard();
  virtual const char* Name(){ return "Standard"; }
  virtual void Initialize(ArduinoJson::JsonDocument& screen);
  virtual void Deinitialize();
};

#endif // __SCREEN_TEMPLATE_GRID_3x1_H__
