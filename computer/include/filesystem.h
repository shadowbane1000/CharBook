#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <stdbool.h>

#include <ArduinoJson.hpp>
#include <memory>
#include <vector> // Required for std::vector
#include <WString.h> // Required for String if not already included by Arduino.h via other headers

extern "C" {
  #include "lauxlib.h"
}

extern "C"{
  bool FS_Open(FS_File* file, const char* filename, const char* mode);
  bool FS_Exists(const char* filename);
}

class FileSystem
{
public:
  static const int MAX_FILENAME_LENGTH = 8;

  void Initialize();
  void Shutdown();
  bool Available();
  bool Exists(const char* filename);
  uint8_t* Load(const char* filename, uint32_t* size);
  bool Save(const char* filename, String body);
  bool LoadJson(const char* filename, ArduinoJson::JsonDocument* document);
  bool SaveJson(const char* filename, ArduinoJson::JsonDocument* document);
  void ChDir(const char* newRoot);
  std::vector<String> MatchFiles(const char* pattern);
  void RegisterLua();
  void Remove(const char* filename);

private:
  char root[MAX_FILENAME_LENGTH + 1] = {0};
  friend bool FS_Open(FS_File* file, const char* filename, const char* mode);
  friend bool FS_Exists(const char* filename);
};

extern class FileSystem FileSystem;

#endif // __FILESYSTEM_H__
