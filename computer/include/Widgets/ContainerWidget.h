#ifndef __CONTAINER_WIDGET_H__
#define __CONTAINER_WIDGET_H__

#include <vector>
#include <ArduinoJson.h>

#include "Widget.h"


class ContainerWidget : public Widget
{
public:
  ContainerWidget() : Widget() {}
  ContainerWidget(JsonObject config) : Widget(config) {}
  std::vector<Widget*> Children; 

  virtual ~ContainerWidget()
  {
    Clear();
  }

  virtual void UpdateChildren(){
    for(Widget* child : Children)
    {
      child->Update();
    }
  }

  virtual void RenderChildren(int16_t parentx, int16_t parenty);

  virtual bool TouchChildren(uint16_t x, uint16_t y)
  {
    x -= GetX();
    y -= GetY();
    for(Widget* child : Children)
    {
      if(child->Touch(x, y)) return true;
    }
    return false;
  }

  virtual void LayoutChildren()
  {
    for(Widget* child : Children)
    {
      child->Layout();
    }
  }

  // pass it along. If you override this, be sure to call the base class
  // if you don't handle the touch.  That way children can.
  virtual bool HandleTouch(uint16_t x, uint16_t y) { 
    return TouchChildren(x, y); 
  }

  // this is called in the update loop and can do minor updates
  virtual void Update(){ UpdateChildren(); }

  // this should redraw the whole screen
  virtual void Render(int16_t parentx, int16_t parenty){ RenderChildren(parentx, parenty); }

  virtual void Layout(){ LayoutChildren(); }

  void Clear()
  {
    for(auto child : Children)
    {
      delete child;
    }
    Children.clear();
  }
};

#endif // __CONTAINER_WIDGET_H__
