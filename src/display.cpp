#include <Arduino.h>
#include <epd_driver.h>

#include "display.h"
#include "image.h"


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
}

void Display::Clear(uint8_t color)
{
  uint8_t fourBits = color & 0x0F;
  memset(framebuffer, fourBits | (fourBits << 4), EPD_WIDTH * EPD_HEIGHT / 2);
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
}

void Display::InvalidateAll()
{
  invalidArea.x = 0;
  invalidArea.y = 0;
  invalidArea.width = EPD_WIDTH;
  invalidArea.height = EPD_HEIGHT;
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
}

Module::UpdateResponse Display::Update()
{
  if(invalidArea.width > 0 && invalidArea.height > 0){

  }

  return SLEEP_OKAY;
}
