#ifndef __BUTTON_WIDGET_H__
#define __BUTTON_WIDGET_H__

#include <Arduino.h>

#include "sizablewidget.h"

class ButtonWidget : public SizableWidget
{
public:
  String text;
  virtual void Render();
  virtual void Update() {}
};

#endif // __BUTTON_WIDGET_H__
