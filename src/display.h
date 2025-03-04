#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>
#include "service.h"

class Image;

class Display : public Module
{
public:
  struct Region {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
  };

  void Initialize();
  virtual Module::UpdateResponse Update();

  void Flush();
  void Clear(uint8_t color);
  void DrawImage(Image* image, int x, int y);
  void InvalidateAll();
  void InvalidateRegion(Region region);

  uint16_t Width;
  uint16_t Height;

private:
  uint8_t* framebuffer;
  Region invalidArea;
};

extern Display Display;

#endif // __DISPLAY_H__
