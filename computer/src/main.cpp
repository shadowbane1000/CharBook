
#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino_DebugUtils.h>

#include "filesystem.h"
#include "display.h"
#include "wireless.h"
#include "batterymonitor.h"
#include "nfc.h"
#include "lua_env.h"
#include "screen.h"
#include "config.h"
#include "version.h"

class Screen Screen;
class Config Config;
class Display Display;
class Wireless Wireless;
class FileSystem FileSystem;
class BatteryMonitor BatteryMonitor;
class Nfc Nfc;
class LuaEnv LuaEnv;

extern "C"
{
void debugprint(const char* string, uint32_t val)
{
  Serial.printf("%s %d\r\n", string, val);
  Serial.flush();
}
void panic(const char* string){
  Serial.println(string);
  Serial.flush();
  //Display.ClearImmediate(0xF);
  //Display.CursorX = 0;
  //Display.CursorY = 100;
  //Display.DrawTextImmediate(string);
  //FileSystem.Shutdown();
  while(1){
    Serial.println(string);
    Serial.flush();
    delay(10000);
  } 
}
void ps_free(void* ptr)
{
  free(ptr);
}
}

void setup()
{
  Serial.begin(115200);
  //Debug.setDebugLevel(DBG_VERBOSE);

  Display.Initialize();
  Display.CursorX = 100;
  Display.CursorY = 100;

  Display.DrawTextImmediate("Version:" VERSION);
  Display.CursorX = 100;
  Display.CursorY = 140;
  Display.DrawTextImmediate("Waiting for serial connection...");
  while(!Serial) delay(100);

  FileSystem.Initialize();
  Config.Initialize();
  Wireless.Initialize();
  BatteryMonitor.Initialize();
  Nfc.Initialize();
  LuaEnv.Initialize();
  FileSystem.RegisterLua();
  Screen.Initialize();
}

void loop()
{
  uint32_t t1 = millis();
  Config.Update();
  Wireless.Update();
  BatteryMonitor.Update();
  Nfc.Update();

  Screen.Update();
  Display.Update();
//  static int i = 0;
//  if(i++ % 1000 == 0) {
//    Serial.printf("Update took %dms\r\n", millis() - t1);
//  }
}

