#ifndef __WIRELESS_WIDGET_H__
#define __WIRELESS_WIDGET_H__

#include "fixedsizewidget.h"

class WirelessWidget : public FixedSizeWidget
{
  void SelectImage(unsigned char** image, unsigned int* size);
  unsigned char* currentImage;
  unsigned int currentImageSize;
public:
  WirelessWidget();
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
  virtual void HandleTouch(uint16_t x, uint16_t y);
};


#endif // __WIRELESS_WIDGET_H__
