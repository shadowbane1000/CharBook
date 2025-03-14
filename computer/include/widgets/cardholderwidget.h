#ifndef __CARD_HOLDER_WIDGET_H__
#define __CARD_HOLDER_WIDGET_H__

#include "containerwidget.h"
#include "display.h"

class CardHolderWidget : public ContainerWidget
{
public:
  CardHolderWidget() { x = 0; y = 80; }
  virtual uint16_t GetWidth() { return Display.Width; }
  virtual uint16_t GetHeight() { return 200*2; }
  virtual uint16_t SetWidth(uint16_t requestedValue){ return GetWidth(); }
  virtual uint16_t SetHeight(uint16_t requestedValue){ return GetHeight(); }

  void Layout();
};

#endif // __CARD_HOLDER_WIDGET_H__
