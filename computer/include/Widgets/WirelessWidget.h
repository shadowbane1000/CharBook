#ifndef __WIRELESS_WIDGET_H__
#define __WIRELESS_WIDGET_H__

#include "Widget.h"

class WirelessWidget : public Widget
{
  void SelectImage(unsigned char** image, unsigned int* size);
  unsigned char* currentImage;
  unsigned int currentImageSize;
public:
  WirelessWidget();
  WirelessWidget(JsonObject config);

  virtual void Update();
  virtual void Render(int16_t parentx, int16_t parenty);
  virtual bool HandleTouch(uint16_t x, uint16_t y);
};


#endif // __WIRELESS_WIDGET_H__
