#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>
#include <stdbool.h>

class Image
{
  friend class Display;
  uint8_t* data;
  uint16_t size;
  bool loaded;
public:
  uint16_t width;
  uint16_t height;

  Image();
  ~Image();
  bool Load(const char* filename);
  void Static(uint8_t* _data, uint16_t _size);
};

#endif // __IMAGE_H__
