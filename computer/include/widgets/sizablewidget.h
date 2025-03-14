#ifndef __SIZABLE_WIDGET_H__
#define __SIZABLE_WIDGET_H__

#include "widget.h"

class SizableWidget : public Widget
{
public:
  virtual uint16_t GetWidth(){ return width; }
  virtual uint16_t GetHeight(){ return height; }
  virtual uint16_t SetWidth(uint16_t requestedValue){ return (width = requestedValue); }
  virtual uint16_t SetHeight(uint16_t requestedValue){ return (height = requestedValue); }

private:
  uint16_t width;
  uint16_t height;
}

#endif // __SIZABLE_WIDGET_H__
