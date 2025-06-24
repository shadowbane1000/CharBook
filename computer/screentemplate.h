#ifndef __SCREEN_TEMPLATE_H__
#define __SCREEN_TEMPLATE_H__

#include "display.h"
#include <vector>

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

class FixedSizeWidget : public Widget
{
public:
  virtual uint16_t SetWidth(uint16_t requestedValue) { return GetWidth(); }
  virtual uint16_t SetHeight(uint16_t requestedValue) { return GetHeight(); }
};

class WidgetContainer : public Widget
{
public:
  std::vector<Widget*> Children; 

  virtual void UpdateChildren(){
    for(Widget* child : Children)
    {
      child->Update();
    }
  }

  virtual void RenderChildren(){
    for(Widget* child : Children)
    {
      child->Render();
    }
  }

  virtual void TouchChildren(uint16_t x, uint16_t y)
  {
    for(Widget* child : Children)
    {
      child->Touch(x, y);
    }
  }

  // pass it along
  virtual void Touch(uint16_t x, uint16_t y) { TouchChildren(x, y); }

  // this is called in the update loop and can do minor updates
  virtual void Update(){ UpdateChildren(); }

  // this should redraw the whole screen
  virtual void Render(){ RenderChildren(); }
};

class ScreenTemplate : public WidgetContainer
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
