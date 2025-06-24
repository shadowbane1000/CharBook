#include <SdFat.h>
#include <SPI.h>

#include "filesystem.h"

static SdFat32 SdFat32;
static bool available = false;

//#define SD_CLK 11
//#define SD_DAT0 16
//#define SD_DAT3 42
//#define SD_CMD 15
//#define SD_CD 9
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

void FileSystem::Initialize()
{
  if(SdFat32.begin(SD_CONFIG)) available = true;
}

bool FileSystem::Available()
{
  return available;
}

bool FileSystem::Exists(const char* filename)
{
  return SdFat32.exists(filename);
}

uint8_t* FileSystem::Load(const char* filename, uint32_t* size)
{
  File32 file;
  if(!file.open(filename, O_RDONLY)) return NULL;
  *size = file.size();
  uint8_t* buffer = (uint8_t*)malloc((*size)+1);
  if(file.fgets((char*)buffer, *size) != *size){
    file.close();
    free(buffer);
    return NULL;
  }
  file.close();
  return (uint8_t*)buffer;
}

bool FileSystem::Save(const char* filename, String body)
{
  File32 file;
  if(!file.open(filename, O_WRONLY|O_CREAT|O_TRUNC)) return false;
  file.write(body.c_str(), body.length());
  file.close();
  return true;
}

bool FileSystem::LoadJson(const char* filename, ArduinoJson::JsonDocument* document)
{
  uint32_t size;
  uint8_t* mybuffer = Load(filename, &size);
  if(mybuffer == NULL) return false;
  ArduinoJson::DeserializationError error = ArduinoJson::deserializeJson(*document, mybuffer);
  free(mybuffer);
  if(error){
    return false;
  }
  return true;
}

bool FileSystem::SaveJson(const char* filename, ArduinoJson::JsonDocument* document)
{
  String result;
  if(ArduinoJson::serializeJsonPretty(*document, result) == 0)
    return false;
  if(!Save(filename, result))
    return false;
  return true;
}

