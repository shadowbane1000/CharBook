#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <stdbool.h>
#include <ArduinoJson.hpp>

class FileSystem
{
public:
  const int MAX_FILENAME_LENGTH = 8;

  void Initialize();
  bool Available();
  bool Exists(const char* filename);
  uint8_t* Load(const char* filename, uint32_t* size);
  bool Save(const char* filename, String body);
  bool LoadJson(const char* filename, ArduinoJson::JsonDocument* document);
  bool SaveJson(const char* filename, ArduinoJson::JsonDocument* document);
};

extern FileSystem FileSystem;

#endif // __FILESYSTEM_H__
