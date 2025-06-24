#include <SPI.h>
#define DISABLE_FS_H_WARNING
#include <SdFat.h>

#include "filesystem.h"
#include "pinout.h"
#include "display.h"
#include "panic.h"
#include "version.h"
#include "lua_env.h" // Added for LuaEnv

// Added for Lua
extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}

SdFs sd;
static bool available = false;

// --- Lua Wrapper Functions for FileSystem ---
static int lua_FileSystem_chdir(lua_State *L) {
    const char* path = luaL_checkstring(L, 1); 
    FileSystem.ChDir(path);
    return 0; 
}

static int lua_FileSystem_findfiles(lua_State *L) {
    const char* pattern = luaL_checkstring(L, 1); 
    
    std::vector<String> matches = FileSystem.MatchFiles(pattern);
    
    lua_newtable(L); 
    int index = 1;
    for (const String& match : matches) {
        lua_pushstring(L, match.c_str());
        lua_rawseti(L, -2, index++); 
    }
    return 1; 
}

void FileSystem::Shutdown()
{
  sd.end();
  SPI.end();
}

void FileSystem::ChDir(const char* newRoot)
{
  strcpy(root, newRoot);
  if(root[strlen(root)-1] == '/')
    root[strlen(root)-1] = '\0';
}

void FileSystem::Initialize()
{
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  SPI.setDataMode(SPI_MODE0);
  bool rlst = sd.begin(SD_CS);
  if (!rlst)
  {
    Serial.println("Failed to init SD card... trying again");
    available = false;
    SPI.end();
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    bool rlst = sd.begin(SD_CS);
    if(!rlst)
      panic("Failed to init SD.");
  }
  Serial.println("SD init success");

  available = true;
}

void FileSystem::Remove(const char* filename)
{
  String path = "/" VERSION;
  if(root[0] != 0) path += "/" + String(root);
  if(filename[0] != '/')
    path += "/";
  path += filename;
  if(sd.exists(path.c_str())) sd.remove(path.c_str());
}

void FileSystem::RegisterLua()
{
  // Register FileSystem class and methods with Lua
  LuaEnv.RegisterClass("FileSystem");
  LuaEnv.RegisterMethod("FileSystem", "chdir", lua_FileSystem_chdir);
  LuaEnv.RegisterMethod("FileSystem", "findfiles", lua_FileSystem_findfiles);
  LuaEnv.RegisterObject("FileSystem", "FileSystem", this); 
}

bool FileSystem::Available()
{
  return available;
}

bool FileSystem::Exists(const char* filename)
{
  String path = "/" VERSION;
  if(root[0] != 0) path += "/" + String(root);
  if(filename[0] != '/')
    path += "/";
  path += filename;
  return sd.exists(path.c_str());
}

uint8_t* FileSystem::Load(const char* filename, uint32_t* size)
{
  String path = "/" VERSION;
  if(root[0] != 0) path += "/" + String(root);
  if(filename[0] != '/')
    path += "/";
  path += filename;
  FsFile file = sd.open(path.c_str(), O_RDONLY);
  if(!file.isOpen())
  {
    return NULL;
  }
  *size = file.fileSize();
  uint8_t* buffer;
  if(*size >= 1024)
  {
    buffer = (uint8_t*)ps_malloc((*size)+1);
  }
  else
  {
    buffer = (uint8_t*)malloc((*size)+1);
  }
  if(file.read(buffer, *size) != *size){
    file.close();
    free(buffer);
    return NULL;
  }
  buffer[*size] = '\0';
  file.close();
  return (uint8_t*)buffer;
}

extern "C" {
  bool FS_Exists(const char* filename)
  {
    String path = "/" VERSION;
    if(FileSystem.root[0] != 0) path += "/" + String(FileSystem.root);
    if(filename[0] != '/')
      path += "/";
    path += filename;
    return sd.exists(path.c_str());
  }

  bool FS_Open(FS_File* file, const char* filename, const char* mode) {
    String path = "/" VERSION;
    if(FileSystem.root[0] != 0) path += "/" + String(FileSystem.root);
    if(filename[0] != '/')
      path += "/";
    path += filename;

    Serial.printf("FS_Open: Opening file %s with mode %s\n", path.c_str(), mode);
    FsFile* internalFile = new FsFile();
    if (!internalFile->open(path.c_str(), mode[0] == 'r' ? O_RDONLY : O_RDWR | O_CREAT | O_TRUNC)) {
      delete internalFile;
      return false;
    }

    *file = internalFile;
    return true;
  }

  int FS_ReadChar(FS_File* file) {
    uint8_t c;
    int result = FS_Read(file, &c, 1);
    if(result == -1) return -1;
    if(result == 0) return -1;
    return c;
  }

  int FS_Read(FS_File* file, uint8_t* buffer, size_t size) {
    FsFile* internalFile = static_cast<FsFile*>(*file);
    if (!internalFile) return -1;
    return internalFile->read(buffer, size);
  }

  int FS_Write(FS_File* file, const uint8_t* buffer, size_t size) {
    FsFile* internalFile = static_cast<FsFile*>(*file);
    if (!internalFile) return -1;
    return internalFile->write(buffer, size);
  }

  int FS_Available(FS_File* file) {
    FsFile* internalFile = static_cast<FsFile*>(*file);
    if (!internalFile) return -1;
    return internalFile->available();
  }

  void FS_Close(FS_File* file) {
    FsFile* internalFile = static_cast<FsFile*>(*file);
    if (internalFile) {
      internalFile->close();
      delete internalFile;
      *file = nullptr;
    }
  }
} // extern "C"

bool FileSystem::Save(const char* filename, String body)
{
  String path = "/" VERSION;
  if(root[0] != 0) path += "/" + String(root);
  if(filename[0] != '/')
    path += "/";
  path += filename;
  if(sd.exists(path.c_str())) sd.remove(path.c_str());
  FsFile file = sd.open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
  if(!file.isOpen())
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
  free(mybuffer);
  if(error){
    Serial.printf("Failed to deserialize file %s  (%s)\r\n", filename, error.c_str());
    Serial.printf("Parsed result: %s\r\n", document->as<String>().c_str());
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

// New method implementation
std::vector<String> FileSystem::MatchFiles(const char* pattern) {
    std::vector<String> foundFiles;
    if (!available) {
        Serial.println("MatchFiles: SD not available.");
        return foundFiles;
    }

    String baseDir = "/" VERSION;
    if(root[0] != 0) {
        baseDir += "/";
        baseDir += String(root);
    }

    String patternArg(pattern);
    String subDirFromPattern = "";
    String filePatternToMatch = patternArg;

    int lastSlashPos = patternArg.lastIndexOf('/');
    if (lastSlashPos != -1) {
        subDirFromPattern = patternArg.substring(0, lastSlashPos);
        filePatternToMatch = patternArg.substring(lastSlashPos + 1);
    }

    String effectiveDir = baseDir;
    if (!subDirFromPattern.isEmpty()) {
        // Append the subdirectory from the pattern.
        // This assumes subDirFromPattern is a relative path segment.
        effectiveDir += "/";
        effectiveDir += subDirFromPattern;
    }

    FsFile dir = sd.open(effectiveDir.c_str());
    if (!dir.isOpen() || !dir.isDirectory()) {
        Serial.printf("MatchFiles: Failed to open directory: %s\n", effectiveDir.c_str());
        return foundFiles;
    }

    FsFile file;
    char fileName[MAX_FILENAME_LENGTH + 10]; // Adjust size as needed for full names from SdFat

    // Simple wildcard matching for '*' at the end or beginning of the pattern part.
    // Example: "*.txt", "prefix*", "*substring*"
    // This is a basic implementation. For full glob support, a more complex matcher is needed.
    String patternStr = filePatternToMatch; // Use the file-only part of the pattern for matching
    bool matchAll = (patternStr == "*");
    bool prefixMatch = (!matchAll && patternStr.endsWith("*"));
    bool suffixMatch = (!matchAll && patternStr.startsWith("*"));
    bool containsMatch = (!matchAll && patternStr.startsWith("*") && patternStr.endsWith("*") && patternStr.length() > 1);
    
    String actualPattern = patternStr;
    if (prefixMatch) actualPattern.remove(actualPattern.length() - 1);
    else if (suffixMatch) actualPattern.remove(0, 1);
    else if (containsMatch) {
        actualPattern.remove(actualPattern.length() - 1);
        actualPattern.remove(0,1);
    }


    while (file.openNext(&dir, O_RDONLY)) {
        if (!file.isDirectory()) {
            file.getName(fileName, sizeof(fileName));
            String sFileName = String(fileName);

            bool matched = false;
            if (matchAll) {
                matched = true;
            } else if (prefixMatch) {
                if (sFileName.startsWith(actualPattern)) matched = true;
            } else if (suffixMatch) {
                if (sFileName.endsWith(actualPattern)) matched = true;
            } else if (containsMatch) {
                if (sFileName.indexOf(actualPattern) != -1) matched = true;
            } else {
                if (sFileName.equals(actualPattern)) matched = true;
            }

            if (matched) {
                foundFiles.push_back(subDirFromPattern + "/" + sFileName);
            }
        }
        file.close();
    }
    dir.close();
    return foundFiles;
}

