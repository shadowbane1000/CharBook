#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
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

  struct Buffer {
    int16_t width;
    int16_t height;
    uint8_t* data;
  };

  typedef enum {
      NO_ROTATION,
      ROTATE_PLUS90,
      ROTATE_MINUS90,
      ROTATE_180
  } Rotation;
  
  typedef enum {
    JUSTIFY_LEFT,
    JUSTIFY_RIGHT,
    JUSTIFY_CENTER
  } Justification;
  
  typedef enum {
    ALIGN_TOP,
    ALIGN_BOTTOM,
    ALIGN_CENTER
  } Alignment;

  int CursorX;
  int CursorY;

  int16_t OffsetX;
  int16_t OffsetY;

  void Initialize();
  virtual Module::UpdateResponse Update();

  void Flush();
  void Clear(uint8_t color);
  void DrawImage(Image* image, int x, int y);
  void DrawPng(const unsigned char* image, const unsigned int size, int x, int y);
  void DrawText(const char* string);
  void DrawText(const Region& area, Justification just, Alignment align, String text, bool transparent_background);
  void SetPixel(int x, int y, uint8_t color); // does not invalidate
  void InvalidateAll();
  void InvalidateRegion(const Region& region);

  // immediate rendering (ignored the frame buffer)
  void ClearImmediate(uint8_t color);
  void DrawTextImmediate(const char* string);

  uint16_t Width;
  uint16_t Height;

private:
  const uint8_t NO_TRANSPARENCY = 255;
  Buffer framebuffer;
  Buffer devicebuffer;

  Region invalidArea;
  unsigned long nextUpdateMillis;
  void BitBlt(
    const Region& sourceRegion, 
    const Region& targetRegion, 
    const Buffer& sourceBuffer, 
    const Buffer& destinationBuffer, 
    Rotation rotation, 
    uint8_t transparentColor);
};

extern class Display Display;

#endif // __DISPLAY_H__
