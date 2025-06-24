
// requirements:
//   boards
//     esp32 by Espressif Systems must be v2.0.17
//   tools
//     ESP32S3 Dev Module
//     USB CDC On Boot: Enabled
//     Flash Size 16MB
//     Partition Scheme: 16MB Flash (3MB App/9.9MB FATFS)
//     PSRAM: "OPI PSRAM"
//   libraries
//     ArduinoJson at version 7.3.1 
//     PNGdec at version 1.1.0 
//     SdFat - Adafruit Fork at version 2.2.54 
//     SPI at version 2.0.0 
//     Arduino_ESP32_OTA at version 0.3.1 
//     Arduino_DebugUtils at version 1.4.0 
//     WiFiClientSecure at version 2.0.0 
//     WiFi at version 2.0.0 
//     ArduinoHttpClient at version 0.6.1 
//     Wire at version 2.0.0 
//     Update at version 2.0.0 

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino_DebugUtils.h>

#include "filesystem.h"
#include "display.h"
#include "wireless.h"
#include "batterymonitor.h"
#include "nfc.h"
#include "lua.h"
#include "screen.h"
#include "config.h"
#include "charbookwebserver.h"
#include "version.h"

class Screen Screen;
class Config Config;
class Display Display;
class Wireless Wireless;
class FileSystem FileSystem;
class BatteryMonitor BatteryMonitor;
class Nfc Nfc;
class Lua Lua;
class CharBookWebServer CharBookWebServer;

extern "C"
{
void debugprint(const char* string, uint32_t val)
{
  Serial.printf("%s %d\r\n", string, val);
  Serial.flush();
}
void panic(const char* string){
  Serial.println(string);
  Display.Clear(0xF);
  Display.Flush();
  Display.CursorX = 0;
  Display.CursorY = 100;
  Display.DrawText(string);
  Serial.flush();
  FileSystem.Shutdown();
  while(1) delay(10000);
}
}

void setup()
{
  Serial.begin(115200);
  Debug.setDebugLevel(DBG_VERBOSE);

  Display.Initialize();
  Display.CursorX = 100;
  Display.CursorY = 100;

  Display.DrawTextImmediate(VERSION);
  while(!Serial) delay(100);

  FileSystem.Initialize();
  Config.Initialize();
  Wireless.Initialize();
  BatteryMonitor.Initialize();
  Nfc.Initialize();
  Lua.Initialize();
  Screen.Initialize();
  CharBookWebServer.Initialize();
}

void loop()
{
  uint32_t t1 = millis();
  Config.Update();
  Wireless.Update();
  BatteryMonitor.Update();
  Nfc.Update();
//  WebServer.Update();

  Screen.Update();
  Display.Update();
}

