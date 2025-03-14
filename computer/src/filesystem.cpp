#include <SPI.h>
#include <SD.h>

#include "filesystem.h"
#include "pinout.h"
#include "display.h"
#include "panic.h"

static bool available = false;

void FileSystem::Shutdown()
{
  SD.end();
  SPI.end();
}

void FileSystem::Initialize()
{
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
  bool rlst = SD.begin(SD_CS, SPI);
  if (!rlst)
  {
    available = false;
    SPI.end();
    panic("Failed to init SD");
  }
  else
  {
    Serial.println("SD init success");
    available = true;
  }
}

bool FileSystem::Available()
{
  return available;
}

bool FileSystem::Exists(const char* filename)
{
  return SD.exists(filename);
}

uint8_t* FileSystem::Load(const char* filename, uint32_t* size)
{
  File file = SD.open(filename, FILE_READ);
  if(!file)
  {
    return NULL;
  }
  *size = file.size();
  uint8_t* buffer = (uint8_t*)malloc((*size)+1);
  if(file.read(buffer, *size) != *size){
    file.close();
    free(buffer);
    return NULL;
  }
  file.close();
  return (uint8_t*)buffer;
}

bool FileSystem::Save(const char* filename, String body)
{
  if(SD.exists(filename)) SD.remove(filename);
  File file = SD.open(filename, FILE_WRITE);
  if(!file)
  {
    return false;
  }
  file.write((uint8_t*)body.c_str(), body.length());
  file.close();
  return true;
}

bool FileSystem::LoadJson(const char* filename, ArduinoJson::JsonDocument* document)
{
  uint32_t size;
  uint8_t* mybuffer = Load(filename, &size);
  if(mybuffer == NULL) {
    Serial.printf("Failed to load file %s\r\n", filename);
    return false;
  }
  ArduinoJson::DeserializationError error = ArduinoJson::deserializeJson(*document, mybuffer);
  Serial.println("file is");
  Serial.write(mybuffer, size);
  Serial.println("Done");
  Serial.printf("template is %s\r\n", (*document)["template"].as<String>().c_str());
  free(mybuffer);
  Serial.printf("template is %s\r\n", (*document)["template"].as<String>().c_str());
  if(error){
    Serial.printf("Failed to deserialize file %s  (%s)\r\n", filename, error.c_str());
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

