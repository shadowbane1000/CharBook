#include <Arduino.h>
#include <ArduinoJson.h>
#include <assert.h>
#include "lua_env.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Helper function to push JSON values to Lua
static void pushJsonToLua(lua_State *L, JsonVariant value)
{
  if (value.is<JsonObject>())
  {
    lua_newtable(L);
    for (JsonPair kv : value.as<JsonObject>())
    {
      lua_pushstring(L, kv.key().c_str());
      pushJsonToLua(L, kv.value());
      lua_settable(L, -3);
    }
  }
  else if (value.is<JsonArray>())
  {
    lua_newtable(L);
    int index = 1;
    for (JsonVariant v : value.as<JsonArray>())
    {
      lua_pushinteger(L, index++);
      pushJsonToLua(L, v);
      lua_settable(L, -3);
    }
  }
  else if (value.is<const char *>())
  {
    lua_pushstring(L, value.as<const char *>());
  }
  else if (value.is<int>())
  {
    lua_pushinteger(L, value.as<int>());
  }
  else if (value.is<float>() || value.is<double>())
  {
    lua_pushnumber(L, value.as<double>());
  }
  else if (value.is<bool>())
  {
    lua_pushboolean(L, value.as<bool>());
  }
  else
  {
    lua_pushnil(L);
  }
}

// Lua wrapper for ArduinoJson parsing
static int lua_ArduinoJson_Parse(lua_State *L)
{
  const char *jsonString = luaL_checkstring(L, 1);
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, jsonString);
  if (error)
  {
    lua_pushnil(L);
    lua_pushstring(L, error.c_str());
    return 2; // Return nil and error message
  }

  pushJsonToLua(L, doc.as<JsonVariant>());
  return 1; // Return the Lua table
}

void LuaEnv::RegisterJsonAsGlobalTable(const char* tableName, ArduinoJson::JsonDocument* jsonDoc)
{
  if (!L || !jsonDoc)
    return;

  lua_newtable(L); // Create a new table
  JsonVariant root = jsonDoc->as<JsonVariant>();
  pushJsonToLua(L, root); // Push JSON data to Lua table
  lua_setglobal(L, tableName); // Set the table as a global variable
}

// Helper function to recursively serialize Lua tables into JSON
static void serializeLuaTableToJson(lua_State *L, JsonDocument &doc, JsonVariant variant)
{
  lua_pushnil(L); // First key
  bool isArray = true;
  int expectedIndex = 1;

  // First pass to determine if the table is an array
  while (lua_next(L, -2) != 0)
  {
    if (!lua_isinteger(L, -2) || lua_tointeger(L, -2) != expectedIndex++)
    {
      isArray = false;
    }
    lua_pop(L, 1); // Remove value, keep key for next iteration
  }

  if (isArray)
  {
    JsonArray array = variant.to<JsonArray>();
    lua_pushnil(L); // Reset iteration
    while (lua_next(L, -2) != 0)
    {
      if (lua_isstring(L, -1))
      {
        array.add(lua_tostring(L, -1));
      }
      else if (lua_isnumber(L, -1))
      {
        array.add(lua_tonumber(L, -1));
      }
      else if (lua_isboolean(L, -1))
      {
        array.add(lua_toboolean(L, -1));
      }
      else if (lua_istable(L, -1))
      {
        JsonVariant nested = array.add<JsonArray>();
        serializeLuaTableToJson(L, doc, nested); // Recursive call for nested table
      }
      else
      {
        lua_pushstring(L, "Unsupported value type in array");
        lua_error(L);
      }
      lua_pop(L, 1); // Remove value, keep key for next iteration
    }
  }
  else
  {
    lua_pushnil(L); // Reset iteration
    while (lua_next(L, -2) != 0)
    {
      const char *key = luaL_checkstring(L, -2);
      if (lua_isstring(L, -1))
      {
        variant[key] = lua_tostring(L, -1);
      }
      else if (lua_isnumber(L, -1))
      {
        variant[key] = lua_tonumber(L, -1);
      }
      else if (lua_isboolean(L, -1))
      {
        variant[key] = lua_toboolean(L, -1);
      }
      else if (lua_istable(L, -1))
      {
        JsonVariant nested = variant[key].to<JsonObject>();
        serializeLuaTableToJson(L, doc, nested); // Recursive call for nested table
      }
      else
      {
        lua_pushstring(L, "Unsupported value type in table");
        lua_error(L);
      }
      lua_pop(L, 1); // Remove value, keep key for next iteration
    }
  }
}

static int lua_ArduinoJson_Serialize(lua_State *L)
{
  luaL_checktype(L, 1, LUA_TTABLE);
  JsonDocument doc;

  JsonVariant root = doc.to<JsonVariant>();
  serializeLuaTableToJson(L, doc, root); // Serialize the Lua table

  char buffer[512];
  size_t len = serializeJson(doc, buffer, sizeof(buffer));
  if (len > 0)
  {
    lua_pushstring(L, buffer);
    return 1; // Return JSON string
  }
  else
  {
    lua_pushnil(L);
    lua_pushstring(L, "Serialization failed");
    return 2; // Return nil and error message
  }
}

void LuaEnv::RegisterGlobalTable(const char *tableName, const char *className)
{
  if (!L)
    return;
  lua_newtable(L);                 // Create a new table
  luaL_getmetatable(L, className); // Get the metatable for the class
  lua_setmetatable(L, -2);         // Set the metatable for the table
  lua_setglobal(L, tableName);     // Set the table as a global variable
}

// Custom print function to override Lua's default print
static int lua_custom_print(lua_State *L) 
{
  int n = lua_gettop(L);  // Number of arguments
  lua_getglobal(L, "tostring"); // Get Lua's tostring function

  for (int i = 1; i <= n; i++) {
    const char *s;
    size_t len;

    lua_pushvalue(L, -1); // Push tostring function
    lua_pushvalue(L, i);  // Push argument i
    lua_call(L, 1, 1);    // Call tostring(arg)
    
    s = lua_tolstring(L, -1, &len); // Get result from tostring
    if (s == NULL) {
      // lua_tolstring pushes an error message if it can't convert.
      // Or, you can use luaL_error for a more formatted Lua error.
      return luaL_error(L, "'tostring' must return a string to 'print'");
    }
    
    if (i > 1) {
      Serial.print("\t"); // Separator, Lua's print uses a tab
    }
    Serial.print(s);
    
    lua_pop(L, 1); // Pop the string result from tostring
  }
  Serial.println();
  lua_pop(L, 1); // Pop the tostring function
  return 0; // print doesn't return any values
}

void LuaEnv::Initialize()
{
  L = luaL_newstate();
  assert(L != NULL);

  luaL_openlibs(L);

  // Register Json class
  RegisterClass("Json");

  // Register Json.Deserialize
  RegisterMethod("Json", "Deserialize", lua_ArduinoJson_Parse);

  // Register Json.Serialize
  RegisterMethod("Json", "Serialize", lua_ArduinoJson_Serialize);

  // Register the global Json table
  RegisterGlobalTable("Json", "Json");

  // override print
  lua_pushcfunction(L, lua_custom_print);
  lua_setglobal(L, "xprint");

  int result = luaL_dofile(L, "/MAIN.LUA");
  if (result == LUA_OK)
  {
    Serial.println("Lua environment initialized and MAIN.LUA executed.");
  }
  else
  {
    const char *error = lua_tostring(L, -1);
    Serial.printf("Failed to execute MAIN.LUA: %s\n", error);
    lua_pop(L, 1); // Pop error message
  }
}

void LuaEnv::Shutdown()
{
  if (L)
  {
    lua_close(L);
    L = NULL;
    Serial.println("Lua environment shut down.");
  }
}

void LuaEnv::RegisterClass(const char *className)
{
  if (!L)
    return;
  luaL_newmetatable(L, className); // Creates metatable and leaves it on stack
  lua_pushvalue(L, -1);            // Duplicate metatable on stack
  lua_setfield(L, -2, "__index");  // Set metatable.__index = metatable
  lua_pop(L, 1);                   // Pop the metatable used for __index
}

void LuaEnv::RegisterMethod(const char *className, const char *methodName, lua_CFunction func)
{
  if (!L)
    return;
  luaL_getmetatable(L, className); // Get metatable from registry
  if (lua_istable(L, -1))
  {
    lua_pushcfunction(L, func);      // Push the C function
    lua_setfield(L, -2, methodName); // Set metatable[methodName] = func
  }
  lua_pop(L, 1); // Pop the metatable
}

void LuaEnv::RegisterObject(const char *objectName, const char *className, void *objectPtr)
{
  if (!L)
    return;
  // Create userdata block and store the C++ object pointer inside it
  void **udata = (void **)lua_newuserdatauv(L, sizeof(void *), 0);
  *udata = objectPtr;

  // Set the metatable for the userdata
  luaL_setmetatable(L, className);

  // Register the userdata as a global variable
  lua_setglobal(L, objectName);
}

LuaValueVariant LuaEnv::ExecuteScript(const char *script)
{
  LuaValueVariant result_variant;
  if (!L)
  {
    result_variant.type = LuaReturnType::ERROR;
    result_variant.error_message = strdup("Lua state is not initialized.");
    return result_variant;
  }

  int initial_stack_top = lua_gettop(L);
  int script_exec_result = luaL_dostring(L, script);

  if (script_exec_result != LUA_OK)
  {
    result_variant.type = LuaReturnType::ERROR;
    const char *err_msg = lua_tostring(L, -1);
    if (err_msg)
    {
      result_variant.error_message = strdup(err_msg);
    }
    else
    {
      result_variant.error_message = strdup("Unknown Lua error.");
    }
    lua_pop(L, 1); // Pop error message
    return result_variant;
  }

  int num_results = lua_gettop(L) - initial_stack_top;

  if (num_results > 0)
  {
    // Process the top-most result
    int lua_type_id = lua_type(L, -1);
    switch (lua_type_id)
    {
    case LUA_TNIL:
      result_variant.type = LuaReturnType::NIL;
      break;
    case LUA_TBOOLEAN:
      result_variant.type = LuaReturnType::BOOLEAN;
      result_variant.value.b_val = lua_toboolean(L, -1);
      break;
    case LUA_TNUMBER:
      result_variant.type = LuaReturnType::NUMBER;
      result_variant.value.d_val = lua_tonumber(L, -1);
      break;
    case LUA_TSTRING:
    {
      result_variant.type = LuaReturnType::STRING;
      const char *s = lua_tostring(L, -1);
      if (s)
      {
        result_variant.value.s_val = strdup(s);
      }
      else
      {
        result_variant.value.s_val = strdup(""); // Should not happen if lua_type is LUA_TSTRING
      }
      break;
    }
    case LUA_TTABLE:
      result_variant.type = LuaReturnType::TABLE;
      // For tables, the variant just indicates the type.
      // Caller would need other means to inspect/use the table if complex data is needed.
      break;
    case LUA_TFUNCTION:
      result_variant.type = LuaReturnType::FUNCTION;
      break;
    case LUA_TUSERDATA:
      result_variant.type = LuaReturnType::USERDATA;
      break;
    case LUA_TTHREAD:
      result_variant.type = LuaReturnType::THREAD;
      break;
    case LUA_TLIGHTUSERDATA:
      result_variant.type = LuaReturnType::LIGHTUSERDATA;
      break;
    default:
      result_variant.type = LuaReturnType::NIL; // Or some UNKNOWN type
      break;
    }
    lua_settop(L, initial_stack_top); // Clear all results from the stack
  }
  else
  {
    // No results returned by the script
    result_variant.type = LuaReturnType::NIL;
    // Stack is already at initial_stack_top or should be restored if dostring changed it without results
    lua_settop(L, initial_stack_top);
  }

  return result_variant;
}
