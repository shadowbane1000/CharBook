#ifndef __LUA_ENV_H__
#define __LUA_ENV_H__

#include <ArduinoJson.h>

#include "lua_variant.h"

// Forward declare lua_State and lua_CFunction
struct lua_State;
typedef int (*lua_CFunction) (lua_State *L);

class LuaEnv
{
public:
  void Initialize();
  void Shutdown();

  // Generic registration functions
  void RegisterClass(const char* className);
  void RegisterMethod(const char* className, const char* methodName, lua_CFunction func);
  void RegisterObject(const char* objectName, const char* className, void* objectPtr);
  void RegisterGlobalTable(const char* tableName, const char* className);
  void RegisterJsonAsGlobalTable(const char* tableName, ArduinoJson::JsonDocument* jsonDoc);
  LuaValueVariant ExecuteScript(const char* script); // Execute a Lua script and return its result

private:
    lua_State *L;

};

extern class LuaEnv LuaEnv;


#endif // __LUA_ENV_H__
