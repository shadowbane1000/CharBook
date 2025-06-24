#ifndef __SCREEN_TEMPLATE_H__
#define __SCREEN_TEMPLATE_H__

#include <ArduinoJson.h>

#include "display.h"
#include "Widgets/ContainerWidget.h"

class ScreenTemplate : public ContainerWidget
{
public:
  ScreenTemplate() : ContainerWidget() 
  {
    SetX(0);
    SetY(0);
  }
  
  // name as chosen in the SCN file.
  virtual const char* Name() = 0;

  virtual void Initialize(JsonDocument& screen)
  {
    SetWidth(Display.Width);
    SetHeight(Display.Height);
  };
  virtual void Deinitialize() = 0;
};

#endif // __SCREEN_TEMPLATE_H__
