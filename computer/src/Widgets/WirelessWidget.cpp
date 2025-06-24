#include <Arduino.h>

#include "wireless.h"
#include "artwork/artwork.h"
#include "Widgets/WirelessWidget.h"
#include "display.h"

WirelessWidget::WirelessWidget()
{
  SetX(0);
  SetY(0);
  SetWidth(80);
  SetHeight(80);
  currentImage = 0;
  currentImageSize = 0;  
}

WirelessWidget::WirelessWidget(JsonObject config) : Widget(config)
{
  // TODO: Initialize WirelessWidget specific members from config if any
}

void WirelessWidget::SelectImage(unsigned char** image, unsigned int* size)
{
  *image = WIFIAVAL_png;
  *size = WIFIAVAL_png_len;
}

bool WirelessWidget::HandleTouch(uint16_t x, uint16_t y)
{
  if(currentImage == WIFIAVAL_png)
  {
    Serial.println("WifiConnect");
    //Render();
    return true;
  }
  return false;
}

void WirelessWidget::Render(int16_t parentx, int16_t parenty)
{
  //Serial.printf("Render %d\r\n", currentImageSize);
  SelectImage(&currentImage, &currentImageSize);
  Display.DrawPng(currentImage, currentImageSize, GetX()+parentx, GetY()+parenty);
}

void WirelessWidget::Update()
{
  unsigned char* newImage;
  unsigned int newImageSize;
  SelectImage(&newImage, &newImageSize);
  if(newImage == currentImage) return;
  Serial.println("Rendering from update");
  //Render();  
}
