#include <Arduino.h>
#include <PNGdec.h>

#include "image.h"
#include "filesystem.h"

extern PNG* png;

Image::Image()
{
  Initialized = false;
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
  //if(png == NULL) png = new(ps_malloc(sizeof(PNG))) PNG();
  if(loaded && data) free(data);
  loaded = false;
  data = _data;
  size = _size;
  png->openRAM(data, size, NULL);
  width = png->getWidth();
  height = png->getHeight();
  png->close();
  Initialized = true;
}

bool Image::Load(const char* filename)
{
  if(loaded && data) free(data);
  if(!FileSystem.Available()){
    Serial.printf("FileSystem not available\r\n");
    return false;
  } 
  if(!FileSystem.Exists(filename)) {
    Serial.printf("File %s not found\r\n", filename);
    return false;
  }
  uint32_t bytesRead;
  data = FileSystem.Load(filename, &bytesRead);
  size = (uint16_t)bytesRead;
  loaded = true;
  Serial.printf("Read PNG size:%d\r\n", size);
  Serial.flush();

  png->openRAM(data, size, NULL);
  width = png->getWidth();
  height = png->getHeight();
  png->close();
  Serial.flush();

  Initialized = true;
  return true;
}
