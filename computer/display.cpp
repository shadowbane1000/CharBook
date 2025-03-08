#include <Arduino.h>
#include "src/LilyGo-EPD47/src/epd_driver.h"
#include <PNGdec.h>

#include "display.h"
#include "image.h"

const uint8_t color_unpack_2_to_4[4] = { 0, 2, 4, 7 };

#define DISPLAY_IDLE_CYCLES 2

typedef struct my_private_struct
{
  int xoff, yoff; // corner offset
} PRIVATE;
PNG png;

void PNGDraw(PNGDRAW *pDraw)
{
  uint8_t* ucMask = (uint8_t*)alloca((pDraw->iWidth + 7) / 8);
  PRIVATE *pPriv = (PRIVATE *)pDraw->pUser;

  assert(pDraw->iPixelType != PNG_PIXEL_TRUECOLOR);
  assert(pDraw->iPixelType != PNG_PIXEL_TRUECOLOR_ALPHA);
  assert(pDraw->iPixelType != PNG_PIXEL_INDEXED);

  if (png.getAlphaMask(pDraw, ucMask, 255)) {
    for(int x = 0; x < pDraw->iWidth; x++)
    {
      // one bit per pixel mask
      if(x > 0 && x % 8 == 0)
        ucMask++;
      if(*ucMask & 0x01)
      {
        uint8_t pixelColor;
        switch(pDraw->iBpp){
        case 8:
          pixelColor = pDraw->pPixels[x] >> 4;
          break;
        case 4:
          pixelColor = pDraw->pPixels[x/2];
          if(x % 2) pixelColor &= 0x0F;
          else pixelColor >> 4;
          break;
        case 2:
          pixelColor = pDraw->pPixels[x/4];
          if(x % 4 == 0) pixelColor >> 6;
          else if(x % 4 == 1) pixelColor >> 4;
          else if(x % 4 == 2) pixelColor >> 2;
          pixelColor = color_unpack_2_to_4[pixelColor & 0x03];
          break;
        case 1:
          pixelColor = pDraw->pPixels[x/8];
          pixelColor >> ((7-x) % 8);
          if(pixelColor & 0x01) pixelColor = 0x0F;
          else pixelColor = 0;
          break;
        }
        Display.SetPixel(pPriv->xoff + x, pPriv->yoff + pDraw->y, pixelColor);
      }
      *ucMask >>= 1;
    }
  }
}

void Display::Initialize()
{
  framebuffer = (uint8_t*)ps_malloc(EPD_WIDTH * EPD_HEIGHT / 2);
  epd_init();
  Clear(0xF);
  Flush();
  Width = EPD_WIDTH;
  Height = EPD_HEIGHT;
}

void Display::Flush()
{
  epd_poweron();
  epd_clear();
  volatile uint32_t t1 = millis();
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  volatile uint32_t t2 = millis();
  Serial.printf("EPD draw took %dms.\r\n", t2 - t1);
  epd_poweroff();

  invalidArea.x = EPD_WIDTH;
  invalidArea.y = EPD_HEIGHT;
  invalidArea.width = -EPD_WIDTH;
  invalidArea.height = -EPD_HEIGHT;
}

void Display::Clear(uint8_t color)
{
  uint8_t fourBits = color & 0x0F;
  memset(framebuffer, fourBits | (fourBits << 4), EPD_WIDTH * EPD_HEIGHT / 2);
  updateCount = 0;
}

void Display::DrawPng(const unsigned char* pngData, const unsigned int length, int x, int y)
{
  int rc = png.openRAM((uint8_t*)pngData, length, PNGDraw);
  if (rc == PNG_SUCCESS) {
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    PRIVATE priv;
    priv.xoff = x;
    priv.yoff = y;
    rc = png.decode((void *)&priv, 0);
    Region region = {
      .x = x,
      .y = y,
      .width = png.getWidth(),
      .height = png.getHeight()
    };
    InvalidateRegion(region);
    png.close();
    updateCount = 0;
  }
}

void Display::DrawImage(Image* image, int x, int y)
{
  Rect_t area = {
      .x = x,
      .y = y,
      .width = image->width,
      .height = image->height
  };
  epd_copy_to_framebuffer(area, image->data, framebuffer);
  updateCount = 0;
}

void Display::InvalidateAll()
{
  invalidArea.x = 0;
  invalidArea.y = 0;
  invalidArea.width = EPD_WIDTH;
  invalidArea.height = EPD_HEIGHT;
  updateCount = 0;
}

void Display::InvalidateRegion(Region region)
{
  uint16_t x1 = invalidArea.x;
  uint16_t y1 = invalidArea.y;
  uint16_t x2 = invalidArea.x + invalidArea.width;
  uint16_t y2 = invalidArea.y + invalidArea.height;

  x1 = x1 < region.x ? x1 : region.x;
  y1 = y1 < region.y ? y1 : region.y;
  x2 = x2 < (region.x + region.width) ? x2 : (region.x + region.width);
  y2 = y2 < (region.y + region.height) ? y2 : (region.y + region.height);
  
  invalidArea.x = x1;
  invalidArea.y = y1;
  invalidArea.width = x2 - x1;
  invalidArea.height = y2 - y1;
  updateCount = 0;
}

void Display::SetPixel(int x, int y, uint8_t color)
{
  epd_draw_pixel(x, y, color, framebuffer);
}

Module::UpdateResponse Display::Update()
{
  if(invalidArea.width > 0 && invalidArea.height > 0){
    if(updateCount++ == DISPLAY_IDLE_CYCLES){
      Flush();
    }
  }

  return SLEEP_OKAY;
}
