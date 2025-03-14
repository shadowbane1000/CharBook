#include <Arduino.h>
#include <WiFi.h>

#include "wireless.h"
#include "config.h"
#include "artwork/artwork.h"
#include "display.h"

void Wireless::Initialize()
{
  State = WIFI_INITIALIZE;
  checkedConfig = false;
}

bool Wireless::WifiPrivate()
{
  int x = rand();
  currentSSID = "CharBook_";
  currentSSID.concat(rand() % 100);
  currentPassword = "CharBook";
  return WiFi.softAP(currentSSID.c_str(), currentPassword.c_str());  
}

String Wireless::GetIPAddress()
{
  return WiFi.localIP().toString();
}

bool Wireless::WifiConnect(const char* networkName)
{
  const char* password = Config.GetString("Wifi", networkName);
  Serial.printf("Connecting to %s with %s...\r\n", networkName, password);
  WiFi.begin(networkName, password);
  do
  {
    auto status = WiFi.status();
    if(status == WL_CONNECTED) {
      Serial.println("Connected");
      return true;
    }
    if(status == WL_CONNECT_FAILED) {
      Serial.println("Connection failed");
      return false;
    }
    delay(100);
  }
  while(1);
}

void Wireless::WifiDisconnect()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool Wireless::WifiScan(std::vector<String>& ScannedNetworks)
{
  int WiFiScanStatus = WiFi.scanNetworks(false);
  if(WiFiScanStatus <= 0) return false;
  ScannedNetworks.clear();
  for(int i=0;i < WiFiScanStatus; i++)
  {
    ScannedNetworks.push_back(WiFi.SSID(i));
    Serial.printf("WiFi scan found network %s\r\n", WiFi.SSID(i));
  }
  WiFi.mode(WIFI_OFF);
  return true;
}

Module::UpdateResponse Wireless::Update()
{
  if(Config.State != Config::SETTINGS_LOADED){
    if(State == WIFI_CONNECTING || State == WIFI_CONNECTED){
      WiFi.disconnect(true);
    }
    State = WIFI_INITIALIZE;
    return SLEEP_OKAY;
  }


  return SLEEP_OKAY;
}

