#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <stdint.h>

class Widget
{
protected:
  uint16_t x,y;
public:
  // position
  uint16_t GetX() { return x; }
  uint16_t GetY() { return y; }
  virtual void SetX(uint16_t requestedValue) { x = requestedValue; }
  virtual void SetY(uint16_t requestedValue) { y = requestedValue; }

  // size of control...   set* might get ignored if the control isn't resizable
  virtual uint16_t GetWidth() = 0;
  virtual uint16_t GetHeight() = 0;
  virtual uint16_t SetWidth(uint16_t requestedValue) = 0;
  virtual uint16_t SetHeight(uint16_t requestedValue) = 0;

  // unless overridden, touches are ignored
  // x,y is relative to our x,y
  virtual void HandleTouch(uint16_t x, uint16_t y) { }

  // filter touch to our bounds:
  virtual void Touch(uint16_t touch_x, uint16_t touch_y) {
    if(touch_x < x || touch_x > x + GetWidth()) return;
    if(touch_y < y || touch_y > y + GetHeight()) return;
    HandleTouch(touch_x - x, touch_y - y);
  }

  // this is called in the update loop and can do minor updates
  virtual void Update() = 0;

  // this should redraw the whole screen
  virtual void Render() = 0;
};

#endif // __WIDGET_H__
