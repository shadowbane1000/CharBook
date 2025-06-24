#include "config.h"
#include "filesystem.h"
#include "service.h"


static ArduinoJson::JsonDocument settings;

void Config::Initialize()
{
  State = STARTUP;
  cardInserted = false;
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
  case ERROR:
    return SLEEP_OKAY;
  case STARTUP:
    State = FileSystem.Exists("DEFAULT.CFG") ? FILE_FOUND : ERROR;
    return SLEEP_OKAY;
  case FILE_FOUND:
    State = FileSystem.LoadJson("DEFAULT.CFG", &settings) ? SETTINGS_LOADED : ERROR;
    return SLEEP_OKAY;
  case SETTINGS_LOADED:
    return SLEEP_OKAY;
  }
  return SLEEP_OKAY;
}

const char* Config::GetString(const char* section, const char* key)
{
  if(State != SETTINGS_LOADED) return NULL;
  return settings[section][key];
}
