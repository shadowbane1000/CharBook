#ifndef __LABEL_H__
#define __LABEL_H__

#include <Arduino.h>
#include <ArduinoJson.h> // Include for JsonObject
#include "Widget.h"
#include "display.h"

class LabelWidget : public Widget
{
public:
  // Constructor from direct parameters
  LabelWidget(const String& text, int16_t x, int16_t y, int16_t w, int16_t h,
              Display::Justification just = Display::JUSTIFY_CENTER,
              Display::Alignment align = Display::ALIGN_CENTER);

  // Constructor from JSON
  LabelWidget(JsonObject config);

  virtual void Render(int16_t parentx, int16_t parenty); // Corrected type int160 -> int16_t
  virtual void Update();
  virtual bool HandleTouch(uint16_t x, uint16_t y) override;

  void SetTransparent(bool transparent) {
    Transparent = transparent;
  }

  String Text;
  Display::Justification Justification;
  Display::Alignment Alignment;
  String m_onTouchScript; // Script to execute on touch
private:
  bool Transparent;
};

#endif // __LABEL_H__
