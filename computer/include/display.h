#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>
#include "module.h"

class Image;

class Display : public Module
{
public:
  struct Region {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
  };
  int CursorX;
  int CursorY;

  void Initialize();
  virtual Module::UpdateResponse Update();

  void Flush();
  void Clear(uint8_t color);
  void DrawImage(Image* image, int x, int y);
  void DrawPng(const unsigned char* image, const unsigned int size, int x, int y);
  void DrawText(const char* string);
  void SetPixel(int x, int y, uint8_t color); // does not invalidate
  void InvalidateAll();
  void InvalidateRegion(Region region);

  // immediate rendering (ignored the frame buffer)
  void ClearImmediate(uint8_t color);
  void DrawTextImmediate(const char* string);

  uint16_t Width;
  uint16_t Height;

private:
  uint8_t* framebuffer;
  Region invalidArea;
  unsigned long nextUpdateMillis;
};

extern Display Display;

#endif // __DISPLAY_H__
