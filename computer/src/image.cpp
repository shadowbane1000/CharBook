#include <Arduino.h>
#include <PNGdec.h>

#include "image.h"
#include "filesystem.h"

Image::Image()
{
  data = NULL;
  loaded = false;
}

Image::~Image()
{
  if(loaded && data)
    free(data);
}

void Image::Static(uint8_t* _data, uint16_t _size)
{
  if(loaded && data) free(data);
  loaded = false;
  data = _data;
  size = _size;
  PNG png;
  png.openRAM(data, size, NULL);
  width = png.getWidth();
  height = png.getHeight();
  png.close();
}

bool Image::Load(const char* filename)
{
  if(loaded && data) free(data);
  if(!FileSystem.Available()) return false;
  if(!FileSystem.Exists(filename)) return false;\
  uint32_t bytesRead;
  data = FileSystem.Load(filename, &bytesRead);
  size = (uint16_t)bytesRead;
  loaded = true;

  PNG png;
  png.openRAM(data, size, NULL);
  width = png.getWidth();
  height = png.getHeight();
  png.close();

  return true;
}
