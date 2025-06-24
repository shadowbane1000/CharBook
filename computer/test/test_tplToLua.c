#include <unity.h>
#include <luatemplate.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

void* ps_malloc(size_t size) { return malloc(size); }
void ps_free(void* ptr) { free(ptr); }
void FS_Close(FS_File* file){}
int FS_Available(FS_File* file){return 0;}
int FS_Write(FS_File* file, const uint8_t* buffer, size_t size){return 0;}
int FS_Read(FS_File* file, uint8_t* buffer, size_t size){return 0;}
int FS_ReadChar(FS_File* file){return 0;}
bool FS_Open(FS_File* file, const char* filename, const char* mode){return false;}

lua_State* state;

void setUp()
{
  state = luaL_newstate();
  luaL_openlibs(state);
}

void tearDown()
{
  lua_close(state);
}

void test_JustAString()
{
  const char* tpl = "Hello, World!";
  const char* expected = "Hello, World!";
  char* result = luaTemplateToLua(tpl);
  luaL_loadstring(state, result);
  lua_pcall(state, 0, 1, 0);
  const char* luaResult = lua_tostring(state, -1);
  lua_pop(state, 1); // Pop the result from the stack
  TEST_ASSERT_EQUAL_STRING(expected, luaResult);
  ps_free(result);
}

void test_ExpressionExpansion()
{
  const char* tpl = "The value is %<value>%";
  const char* expected = "The value is 42";
  char* result = luaTemplateToLua(tpl);
  lua_pushinteger(state, 42);
  lua_setglobal(state, "value");
  luaL_loadstring(state, result);
  lua_pcall(state, 0, 1, 0);
  const char* luaResult = lua_tostring(state, -1);
  lua_pop(state, 1); // Pop the result from the stack
  TEST_ASSERT_EQUAL_STRING(expected, luaResult);
  ps_free(result);
}

void test_StatementExpansion()
{
  const char* tpl = "%{for x=1,2 do}%hi%{end}%";
  const char* expected = "hihi";
  char* result = luaTemplateToLua(tpl);
  printf("Generated Lua script: %s\n", result);
  luaL_loadstring(state, result);
  lua_pcall(state, 0, 1, 0);
  const char* luaResult = lua_tostring(state, -1);
  lua_pop(state, 1); // Pop the result from the stack
  TEST_ASSERT_EQUAL_STRING(expected, luaResult);
  ps_free(result);
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_JustAString);
  RUN_TEST(test_ExpressionExpansion);
  RUN_TEST(test_StatementExpansion);
  RUN_TEST(test_generateGammaTable);
  UNITY_END();
  return 0;
}
