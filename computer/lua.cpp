#include <assert.h>

#include "lua.h"

#include "src/lua-5.4.7/src/lua.h"
#include "src/lua-5.4.7/src/lauxlib.h"
#include "src/lua-5.4.7/src/lualib.h"

static lua_State *L;


void Lua::Initialize()
{
  L = luaL_newstate();
  assert(L != NULL);

  luaL_openlibs(L);
  int result = luaL_dofile(L, "COMMON/MAIN.LUA");
  assert(result == 0);
}
