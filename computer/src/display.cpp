#include <Arduino.h>
#include <epd_driver.h>
#include <PNGdec.h>

#include "display.h"
#include "image.h"

#include "firasans.h"

const uint8_t color_unpack_2_to_4[4] = { 0, 2, 4, 7 };

#define UPDATE_DELAY 500
#define DONT_UPDATE (0xFFFFFFFFu)

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
    uint16_t buffer[pDraw->iWidth];
    png.getLineAsRGB565(pDraw, buffer, 0, -1);
    //Serial.printf("Draw w=%d alpha=%d pitch=%d bpp=%d type=%d\r\n", pDraw->iWidth, pDraw->iHasAlpha, pDraw->iPitch, pDraw->iBpp, pDraw->iPixelType);
    for(int x = 0; x < pDraw->iWidth; x++)
    {
      // one bit per pixel mask
      if(x > 0 && x % 8 == 0)
        ucMask++;
      if(*ucMask & 0x01)
      {
        Display.SetPixel(pPriv->xoff + x, pPriv->yoff + pDraw->y, (buffer[x] >> 1) & 0xF);
      }
      *ucMask >>= 1;
    }
  }
}

void Display::Initialize()
{
  Width = EPD_WIDTH;
  Height = EPD_HEIGHT;
  framebuffer = (uint8_t*)ps_malloc(EPD_WIDTH * EPD_HEIGHT / 2);
  epd_init();
  Clear(0xF);
  Flush();
//  CursorX = 0;
//  CursorY = 100;
//  DrawText("Display Initialized");
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
  nextUpdateMillis = DONT_UPDATE;
}

void Display::ClearImmediate(uint8_t color)
{
  epd_poweron();
  epd_clear();
  epd_poweroff();
}

void Display::Clear(uint8_t color)
{
  uint8_t fourBits = color & 0x0F;
  memset(framebuffer, fourBits | (fourBits << 4), EPD_WIDTH * EPD_HEIGHT / 2);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::DrawText(const char* text)
{
  write_string((GFXfont *)&FiraSans, text, &CursorX, &CursorY, framebuffer);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::DrawTextImmediate(const char* text)
{
  write_string((GFXfont *)&FiraSans, text, &CursorX, &CursorY, NULL);
}

void Display::DrawPng(const unsigned char* pngData, const unsigned int length, int x, int y)
{
  int rc = png.openRAM((uint8_t*)pngData, length, PNGDraw);
  if (rc == PNG_SUCCESS) {
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    Serial.printf("  drawing at %d,%d\r\n", x, y);
    PRIVATE priv;
    priv.xoff = x;
    priv.yoff = y;
    rc = png.decode((void *)&priv, 0);
    Region region;
    region.x = x;
    region.y = y;
    region.width = (uint16_t)png.getWidth();
    region.height = (uint16_t)png.getHeight();
    InvalidateRegion(region);
    png.close();
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
  //epd_copy_to_framebuffer(area, image->data, framebuffer);
  DrawPng(image->data, image->size, x, y);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::InvalidateAll()
{
  invalidArea.x = 0;
  invalidArea.y = 0;
  invalidArea.width = EPD_WIDTH;
  invalidArea.height = EPD_HEIGHT;
  nextUpdateMillis = millis()+UPDATE_DELAY;
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
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::SetPixel(int x, int y, uint8_t color)
{
  //Serial.printf("SetPixel(%d, %d, %x)\r\n", x, y, color);
  epd_draw_pixel(x, y, color << 4, framebuffer);
}

Module::UpdateResponse Display::Update()
{
  if(nextUpdateMillis != DONT_UPDATE){
    if(millis() > nextUpdateMillis){
      Flush();
    }
  }

  return SLEEP_OKAY;
}
