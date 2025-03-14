#include "widgets/cardwidget.h"
#include "display.h"

CardWidget::CardWidget(const char* backgroundImageName)
{
  backgroundImage.Load(backgroundImageName);
}

CardWidget::CardWidget(uint8_t* backgroundImageData, uint16_t backgroundImageSize)
{
  backgroundImage.Static(backgroundImageData, backgroundImageSize);
}

void CardWidget::Render() 
{
  Display.DrawImage(&backgroundImage, x, y);
}
