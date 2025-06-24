#ifndef __SCREEN_TEMPLATE_H__
#define __SCREEN_TEMPLATE_H__

#include "display.h"
#include "widgets/containerwidget.h"

class ScreenTemplate : public ContainerWidget
{
public:
  // name as chosen in the SCN file.
  virtual const char* Name() = 0;

  virtual uint16_t GetWidth() { return Display.Width; }
  virtual uint16_t GetHeight() { return Display.Height; }
  virtual uint16_t SetWidth(uint16_t requestedValue) { return Display.Width; }
  virtual uint16_t SetHeight(uint16_t requestedValue) { return Display.Height; }

};

#endif // __SCREEN_TEMPLATE_H__
