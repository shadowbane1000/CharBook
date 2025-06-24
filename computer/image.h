#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>
#include <stdbool.h>

class Image
{
  friend class Display;
  uint8_t* data;
public:
  uint16_t width;
  uint16_t height;

  Image();
  ~Image();
  bool Load(const char* filename);
};

#endif // __IMAGE_H__
