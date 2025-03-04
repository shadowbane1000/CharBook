#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include "filesystem.h"
#include "display.h"
#include "wireless.h"
#include "batterymonitor.h"
#include "nfc.h"
#include "lua.h"
#include "screen.h"
#include "ota.h"
#include "config.h"

class Screen Screen;
class Config Config;
class Display Display;
class Wireless Wireless;
class FileSystem FileSystem;
class BatteryMonitor BatteryMonitor;
class Nfc Nfc;
class Lua Lua;
class Ota Ota;

void setup()
{
  Serial.begin(115200);

  FileSystem.Initialize();
  Config.Initialize();
  Display.Initialize();
  Wireless.Initialize();
  BatteryMonitor.Initialize();
  Nfc.Initialize();
  Lua.Initialize();
  Ota.Initialize();
  Screen.Initialize();
}

void loop()
{
  uint32_t t1 = millis();
  Config.Update();
  Wireless.Update();
  Ota.Update();
  BatteryMonitor.Update();
  Nfc.Update();

  Screen.Update();
  Display.Update();
  
  uint32_t t2 = millis();
  uint32_t updateTime = t2 - t1;
  if(updateTime < 500) delay(500 - updateTime);
}

