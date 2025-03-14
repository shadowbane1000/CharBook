#ifndef __CARD_WIDGET_H__
#define __CARD_WIDGET_H__

#include "fixedsizewidget.h"
#include "image.h"

class CardWidget : public FixedSizeWidget
{
public:
  CardWidget(const char* backgroundImageName);
  CardWidget(uint8_t* backgroundImageData, uint16_t backgroundImageSize);

  virtual uint16_t GetWidth() { return 200; }
  virtual uint16_t GetHeight() { return 200; }
  virtual void Render();
  virtual void Update(){}

private:
  Image backgroundImage;
};

#endif // __CARD_WIDGET_H__
