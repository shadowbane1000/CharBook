#include "luatemplate.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define bufferSize (16*1024)

extern void* ps_malloc(size_t size);
extern void ps_free(void* ptr);

struct state
{
  char* luaScript;
  char* currPos;
  char* currentText;
  char* currPosText;
};

static bool initState(struct state* state)
{
  state->luaScript = (char *)ps_malloc(bufferSize);
  if (state->luaScript == NULL)
  {
    return false;
  }
  state->currPos = state->luaScript;
  state->currentText = (char *)ps_malloc(bufferSize);
  if (state->currentText == NULL)
  {
    ps_free(state->luaScript);
    return false;
  }
  state->currPosText = state->currentText;
  return true;
} 

static char* freeState(struct state* state)
{
  if (state->currentText != NULL)
  {
    ps_free(state->currentText);
  }
  return state->luaScript;
}

static void addToLuaScript(struct state* state, const char* str)
{
  strcpy(state->currPos, str);
  state->currPos += strlen(str);
  assert(state->currPos < state->luaScript + bufferSize);
}

static bool isCurrentTextEmpty(struct state* state)
{
  return state->currPosText == state->currentText;
}

static void addCharToText(struct state* state, char c)
{
  *state->currPosText++ = c;
  *state->currPosText = '\0';
  assert(state->currPosText < state->currentText + bufferSize);
}

static void flushCurrentText(struct state* state)
{
  if (!isCurrentTextEmpty(state))
  {
    addToLuaScript(state, "str = str .. [[");
    addToLuaScript(state, state->currentText);
    addToLuaScript(state, "]]\n");
    state->currPosText = state->currentText;
    *state->currPosText = '\0';
  }
}

static void flushCurrentTextRaw(struct state* state)
{
  if (!isCurrentTextEmpty(state))
  {
    addToLuaScript(state, state->currentText);
    state->currPosText = state->currentText;
    *state->currPosText = '\0';
  }
}

char *luaTemplateToLua(const char *tpl)
{
  struct state state;
  if (!initState(&state))
  {
    return NULL;
  }

  addToLuaScript(&state, "local str = \"\"\n");

  int i = 0;
  if (tpl == NULL){
    addToLuaScript(&state, "return str -- Error: tpl input was null\n");
    return freeState(&state);
  }
  int len = strlen(tpl);

  while (i < len)
  {
    if (tpl[i] == '%')
    {
      if (i + 1 < len)
      {
        if (tpl[i + 1] == '%')
        {
          addCharToText(&state, '%'); // Treat '%%' as a literal '%'
          i += 2;                           // Consume both '%'
          continue;
        }
        else if (tpl[i + 1] == '{')
        {         // Statement block %{...}%
          flushCurrentText(&state);
          i += 2; // Skip '%{'
          while (i < len)
          {
            if (tpl[i] == '}' && (i + 1 < len && tpl[i + 1] == '%'))
            {
              i += 2; // Skip '}%'
              break;
            }
            addCharToText(&state, tpl[i]);
            i++;
          }
          if (i >= len && !(tpl[len - 2] == '}' && tpl[len - 1] == '%'))
          { // Unterminated block
            addToLuaScript(&state, "-- Error: Unterminated statement block\n");
            return freeState(&state);
          }
          addCharToText(&state, '\n');
          flushCurrentTextRaw(&state);
        }
        else if (tpl[i + 1] == '<')
        {         // Expression block %<...>%
          flushCurrentText(&state);
          i += 2; // Skip '%<'
          while (i < len)
          {
            if (tpl[i] == '>' && (i + 1 < len && tpl[i + 1] == '%'))
            {
              i += 2; // Skip '>%'
              break;
            }
            addCharToText(&state, tpl[i]);
            i++;
          }
          if (i >= len && !(tpl[len - 2] == '>' && tpl[len - 1] == '%'))
          { // Unterminated block
            addToLuaScript(&state, "-- Error: Unterminated expression block\n");
            return freeState(&state);
          }
          addToLuaScript(&state, "str = str .. tostring(");
          flushCurrentTextRaw(&state);
          addToLuaScript(&state, ")\n");
        }
        else
        {                     // other character following '%'
          addCharToText(&state, '%'); // Treat '%' as a literal character
          i++;                // Consume only the '%'
        }
      }
      else
      { // '%' is the last character in the string
        addCharToText(&state, '%');
        i++;
      }
    }
    else
    {
      addCharToText(&state, tpl[i]);
      i++;
    }
  }

  flushCurrentText(&state);

  addToLuaScript(&state, "return str\n");
  return freeState(&state);
}
