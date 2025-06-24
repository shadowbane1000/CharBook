#include <Arduino.h>
#include <SdFat.h>

#include "image.h"
#include "filesystem.h"

Image::Image()
{
  data = NULL;
}

Image::~Image()
{
  if(data)
    free(data);
}

bool Image::Load(const char* filename)
{
  if(data) free(data);
  if(!FileSystem.Available()) return false;
  if(!FileSystem.Exists(filename)) return false;\
  uint32_t bytesRead;
  data = FileSystem.Load(filename, &bytesRead);
}
