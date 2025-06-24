#include <WiFi.h>

#include "wireless.h"
#include "ota.h"
#include "config.h"
#include "src/artwork/artwork.h"

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
    WiFi.mode(WIFI_OFF);
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

WirelessButton::WirelessButton()
{
  x = 0;
  y = 0;
  currentImage = 0;
  currentImageSize = 0;  
}

void WirelessButton::SelectImage(unsigned char** image, unsigned int* size)
{
  switch(Wireless.State){
  case Wireless::WIFI_UNAVAILABLE:
    *image = WIFIUNAV_png;
    *size = WIFIUNAV_png_len;
    break;
  case Wireless::WIFI_AVAILABLE:
    *image = WIFIAVAL_png;
    *size = WIFIAVAL_png_len;
    break;
  case Wireless::WIFI_CONNECTING:
    *image = WIFICONN_png;
    *size = WIFICONN_png_len;
    break;
  case Wireless::WIFI_CONNECTED:
    switch(Ota.State){
      case Ota::IDLE:
      case Ota::NO_UPDATE:
        *image = WIFION_png;
        *size = WIFION_png_len;
        break;
      case Ota::UPDATE_READY:
        *image = UPDATE_png;
        *size = UPDATE_png_len;
        break;
      case Ota::UPDATE_AVAILABLE:
        *image = WIFIOTAD_png;
        *size = WIFIOTAD_png_len;
        break;
      case Ota::DOWNLOADING:
      case Ota::GETTING_VERSIONS_FILE:
      case Ota::INITIALIZING:
        *image = WIFIOTAW_png;
        *size = WIFIOTAW_png_len;
        break;
      case Ota::ERROR:
      default:
        *image = ERROR_png;
        *size = ERROR_png_len;
        break;
    }
    break;
  default:
    *image = ERROR_png;
    *size = ERROR_png_len;
    break;
  }
}

void WirelessButton::HandleTouch(uint16_t x, uint16_t y)
{
  if(currentImage == UPDATE_png)
  {
    Serial.print("Applying Update");
    Serial.println();
    Ota.ApplyUpdate();
    Serial.print("Update failed");
    Serial.println();
    Render();
    return;
  }

  if(currentImage == WIFIOTAD_png)
  {
    Ota.DownloadUpdate();
    Render();
    return;
  }

  if(currentImage == WIFIAVAL_png)
  {
    Wireless.WifiConnect();
    Render();
    return;
  }

  if(currentImage == WIFICONN_png || currentImage == WIFION_png)
  {
    Wireless.WifiDisconnect();
    return;
  }
}

void WirelessButton::Render()
{
  SelectImage(&currentImage, &currentImageSize);
  Display.DrawPng(WIFIUNAV_png, WIFIUNAV_png_len, x, y);
}

void WirelessButton::Update()
{
  unsigned char* newImage;
  unsigned int newImageSize;
  SelectImage(&newImage, &newImageSize);
  if(newImage == currentImage) return;
  Render();  
}
