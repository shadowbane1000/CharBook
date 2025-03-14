#include "fixedsizewidget.h"

class SettingsWidget : public FixedSizeWidget
{
public:
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
  virtual void HandleTouch(uint16_t x, uint16_t y);

};
