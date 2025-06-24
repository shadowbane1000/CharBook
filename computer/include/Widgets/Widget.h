#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <stdint.h>
#include <ArduinoJson.h>

class Widget
{
private:
  int16_t x,y;
  int16_t w,h;
  enum {
    FLAG_POSITION_CHANGED = 0x01,
    FLAG_SIZE_CHANGED = 0x02,
  };
  uint8_t flags;
public:
  Widget() : x(0), y(0), w(0), h(0), flags(0) {}
  Widget(JsonObject config) {}
  virtual ~Widget(){}
  
  // position
  int16_t GetX() { return x; }
  int16_t GetY() { return y; }
  void SetX(int16_t requestedValue) { x = requestedValue; flags |= FLAG_POSITION_CHANGED; }
  void SetY(int16_t requestedValue) { y = requestedValue; flags |= FLAG_POSITION_CHANGED; }

  // size of control...   set* might get ignored if the control isn't resizable
  int16_t GetWidth() { return w; }
  int16_t GetHeight() { return h; }
  void SetWidth(int16_t requestedValue) { w = requestedValue; flags |= FLAG_SIZE_CHANGED; }
  void SetHeight(int16_t requestedValue) { h = requestedValue; flags |= FLAG_SIZE_CHANGED; }

  // unless overridden, touches are ignored
  // x,y is relative to our x,y
  virtual bool HandleTouch(uint16_t x, uint16_t y) { return false; }

  // filter touch to our bounds:
  virtual bool Touch(uint16_t touch_x, uint16_t touch_y) {
    if(touch_x < GetX() || touch_x > GetX() + GetWidth()) return false;
    if(touch_y < GetY() || touch_y > GetY() + GetHeight()) return false;
    Serial.printf("Touch at %d,%d in %s\n", touch_x, touch_y, __PRETTY_FUNCTION__);
    return HandleTouch(touch_x - GetX(), touch_y - GetY());
  }

  virtual void Layout(){}

  // this is called in the update loop and can do minor updates
  virtual void Update() = 0;

  // this should redraw the whole screen
  virtual void Render(int16_t parentx, int16_t parenty) = 0;
};

#endif // __WIDGET_H__
