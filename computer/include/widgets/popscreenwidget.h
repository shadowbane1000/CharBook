#ifndef __POP_SCREEN_WIDGET_H__
#define __POP_SCREEN_WIDGET_H__

#include "fixedsizewidget.h"

class PopScreenWidget : public FixedSizeWidget
{
public:
  PopScreenWidget() { x = 80; y = 0; }
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
  virtual void HandleTouch(uint16_t x, uint16_t y);

};

#endif // __POP_SCREEN_WIDGET_H__
