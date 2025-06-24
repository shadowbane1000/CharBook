#ifndef __FIX_SIZED_WIDGET_H__
#define __FIX_SIZED_WIDGET_H__

#include "widget.h"

class FixedSizeWidget : public Widget
{
public:
  virtual uint16_t SetWidth(uint16_t requestedValue) { return GetWidth(); }
  virtual uint16_t SetHeight(uint16_t requestedValue) { return GetHeight(); }
};

#endif // __FIX_SIZED_WIDGET_H__
