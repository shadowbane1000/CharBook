#include "rom/rtc.h"
#include "config.h"
#include "filesystem.h"
#include "src/module.h"
#include "src/artwork/artwork.h"


static ArduinoJson::JsonDocument settings;

void Config::Initialize()
{
  State = STARTUP;
  cardInserted = false;
  saveDelay = 0;
}

Module::UpdateResponse Config::Update()
{
  if(!FileSystem.Available()) 
  {
    cardInserted = false;
    return SLEEP_OKAY;
  }
  else if (!cardInserted){
    State = STARTUP;
    cardInserted = true;
  }

  switch(State)
  {
  default:
  case ERROR:
    return SLEEP_OKAY;
  case STARTUP:
    State = FileSystem.Exists("DEFAULT.CFG") ? FILE_FOUND : ERROR;
    return SLEEP_OKAY;
  case FILE_FOUND:
    State = FileSystem.LoadJson("DEFAULT.CFG", &settings) ? SETTINGS_LOADED : ERROR;
    return SLEEP_OKAY;
  case SETTINGS_LOADED:
    break;
  }
  if(saveDelay < UPDATES_BEFORE_FLUSH){
    saveDelay++;
    return STAY_AWAKE;
  } 
  else if (saveDelay == UPDATES_BEFORE_FLUSH)
  {
    if(FileSystem.SaveJson("DEFAULT.CFG", &settings))
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

void SettingsButton::Update()
{
}

void SettingsButton::Render()
{
  Display.DrawPng(SETTING_png, SETTING_png_len, x, y);
}

