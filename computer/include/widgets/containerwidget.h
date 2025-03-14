#ifndef __CONTAINER_WIDGET_H__
#define __CONTAINER_WIDGET_H__

#include <vector>

#include "widget.h"


class ContainerWidget : public Widget
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

#endif // __CONTAINER_WIDGET_H__
