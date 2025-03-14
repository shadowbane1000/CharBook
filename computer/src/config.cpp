#include <Arduino.h>

#include "rom/rtc.h"
#include "config.h"
#include "filesystem.h"
#include "module.h"
#include "panic.h"
#include "display.h"

static ArduinoJson::JsonDocument settings;

void Config::Initialize()
{
  State = STARTUP;
  cardInserted = FileSystem.Available();
  saveDelay = 0;
  Display.CursorX=100;
  Display.CursorY=140;
  if(cardInserted)
  {
    bool loadDefault = true;
    if(FileSystem.Exists("/CURRENT.CFG"))
    {
      if(FileSystem.LoadJson("/CURRENT.CFG", &settings))
      {
        State = SETTINGS_LOADED;
        Serial.printf("Loaded CURRENT.CFG\r\n");
        return;
      }
      else 
      {
        Serial.print("current.cfg exists, but is invalid... loading default.cfg");
      }
    }

    if(loadDefault && FileSystem.Exists("/DEFAULT.CFG"))
    {
      if(FileSystem.LoadJson("/DEFAULT.CFG", &settings))
      {
        State = SETTINGS_LOADED;
        Serial.printf("Loaded DEFAULT.CFG\r\n");
      } 
      else 
      {
        State = ERROR;
        panic("Failed to load DEFAULT.CFG (bad format?)\r\n");
      }
    }
    else
    {
      State = ERROR;
      panic("Card inserted, but DEFAULT.CFG doesn't exist\r\n");
    }
  } 
  else 
  {
    Serial.printf("No Card inserted\r\n");
    panic("No card inserted");
  }
}

Module::UpdateResponse Config::Update()
{
  if(!FileSystem.Available()) 
  {
    cardInserted = false;
    return SLEEP_OKAY;
  }
  else if (!cardInserted)
  {
    State = STARTUP;
    cardInserted = true;
    Initialize();
    return SLEEP_OKAY;
  }

  if(saveDelay < UPDATES_BEFORE_FLUSH){
    saveDelay++;
    return STAY_AWAKE;
  } 
  else if (saveDelay == UPDATES_BEFORE_FLUSH)
  {
    if(FileSystem.SaveJson("/CURRENT.CFG", &settings))
    {
      saveDelay++;
    } else {
      State = ERROR;
      return SLEEP_OKAY;
    }
  }
  return SLEEP_OKAY;
}

void Config::SetArray(const char* section, const char* key, char* values, uint16_t valueLength, uint16_t valueCount)
{
  assert(State == SETTINGS_LOADED);
  ArduinoJson::JsonArray array;
  for(int i=0; i < valueCount;i++, values += valueLength)
  {
    array[i] = values;
  }
  settings[section][key] = array;
  saveDelay = 0;
}

const char* Config::GetString(const char* section, const char* key)
{
  assert(State == SETTINGS_LOADED);
  if(State != SETTINGS_LOADED) return NULL;
  return settings[section][key];
}
