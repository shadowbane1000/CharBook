#include <WiFi.h>

#include "wireless.h"
#include "config.h"

void Wireless::Initialize()
{
  State = WIFI_UNAVAILABLE;
  checkedConfig = false;
}

void Wireless::WifiConnect()
{
  if(Config.State != Config::SETTINGS_LOADED) return;
  if(State != WIFI_AVAILABLE) return;
  State = WIFI_CONNECTING;
  return;
}

void Wireless::WifiDisconnect()
{
  if(State == WIFI_CONNECTING || State == WIFI_CONNECTED){
    WiFi.disconnect(true);
    State = WIFI_AVAILABLE;
  }
}

Module::UpdateResponse Wireless::Update()
{
  if(Config.State != Config::SETTINGS_LOADED){
    if(State == WIFI_CONNECTING || State == WIFI_CONNECTED){
      WiFi.disconnect(true);
    }
    State = WIFI_UNAVAILABLE;
    return SLEEP_OKAY;
  }

  if(State == WIFI_CONNECTING)
  {
    if(WiFi.begin(Config.GetString("Wifi", "SSID"), Config.GetString("Wifi", "Password")) == WL_CONNECTED)
    {
      State = WIFI_CONNECTED;
    }
    return STAY_AWAKE;
  }

  if(State == WIFI_CONNECTED){
    return STAY_AWAKE;
  }

  return SLEEP_OKAY;
}
