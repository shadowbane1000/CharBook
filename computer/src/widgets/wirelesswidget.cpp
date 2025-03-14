#include <Arduino.h>

#include "wireless.h"
#include "artwork/artwork.h"
#include "widgets/wirelesswidget.h"
#include "display.h"

WirelessWidget::WirelessWidget()
{
  x = 0;
  y = 0;
  currentImage = 0;
  currentImageSize = 0;  
}

void WirelessWidget::SelectImage(unsigned char** image, unsigned int* size)
{
  *image = WIFIAVAL_png;
  *size = WIFIAVAL_png_len;
}

void WirelessWidget::HandleTouch(uint16_t x, uint16_t y)
{
  if(currentImage == WIFIAVAL_png)
  {
    Serial.println("WifiConnect");
    Render();
    return;
  }
}

void WirelessWidget::Render()
{
  //Serial.printf("Render %d\r\n", currentImageSize);
  SelectImage(&currentImage, &currentImageSize);
  Display.DrawPng(currentImage, currentImageSize, x, y);
}

void WirelessWidget::Update()
{
  unsigned char* newImage;
  unsigned int newImageSize;
  SelectImage(&newImage, &newImageSize);
  if(newImage == currentImage) return;
  Serial.println("Rendering from update");
  Render();  
}
