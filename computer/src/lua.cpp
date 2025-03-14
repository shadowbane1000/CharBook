#include <Arduino.h>

#include <assert.h>

#include "lua.h"

#include "lua-5.4.7/src/lua.h"
#include "lua-5.4.7/src/lauxlib.h"
#include "lua-5.4.7/src/lualib.h"

static lua_State *L;


void Lua::Initialize()
{
  L = luaL_newstate();
  assert(L != NULL);

  luaL_openlibs(L);
  int result = luaL_dofile(L, "COMMON/MAIN.LUA");
  if(result == 0)
  {
    Serial.printf("Lua is ready to go\r\n", result);
  }
  else 
  {
      Serial.printf("dofile of COMMON/MAIN.LUA resulted in %d\r\n", result);
  }
}
