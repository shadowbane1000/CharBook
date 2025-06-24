#include <Arduino.h>
#include <Arduino_ESP32_OTA.h>
#include <ArduinoJson.hpp>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "ota.h"
#include "wireless.h"
#include "config.h"
#include "src/module.h"

static Arduino_ESP32_OTA* ota;
#define MAX_URL_SIZE 256
static char* url;
WiFiClientSecure client;

const char* Ota::GetVersion()
{
  return "0.0.1";
}

void Ota::Initialize()
{
  url = (char*)malloc(MAX_URL_SIZE);
  if(!url) while(1);
  ota = new Arduino_ESP32_OTA();
  State = IDLE;
}

void Ota::DownloadUpdate()
{
  if(State == UPDATE_AVAILABLE) State = INITIALIZING;
}

void Ota::ApplyUpdate()
{
  if(State != UPDATE_READY) return;

  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;
  if((ota_err = ota->update()) != Arduino_ESP32_OTA::Error::None){
    State = ERROR;
    return;
  }
  // reset to new version...
  ota->reset();
  while(1);
}

Module::UpdateResponse Ota::Update()
{
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;
  int downloadResult;

  if(State == NO_UPDATE) return SLEEP_OKAY;

  if(Config.State != Config::SETTINGS_LOADED){
    State = IDLE;
    return SLEEP_OKAY;
  }

  if(State < UPDATE_READY && Wireless.State != Wireless::WIFI_CONNECTED){
    State = IDLE;
    return SLEEP_OKAY;
  }

  switch(State){
  case IDLE:
    State = GETTING_VERSIONS_FILE;
    strcpy(url, Config.GetString("Ota", "Url"));
    strcat(url, "VERSIONS.CFG");
    xTaskCreate(
      DownloadUpdateListTask,
      "DownloadUpdateList",
      1024,
      NULL,1,NULL);
    State = GETTING_VERSIONS_FILE;
    return SLEEP_OKAY;
  case GETTING_VERSIONS_FILE:
    return SLEEP_OKAY;
  case INITIALIZING:
    if(Wireless.State != Wireless::WIFI_CONNECTED) {
      State = IDLE;
      return SLEEP_OKAY;
    }
    if((ota_err = ota->begin()) != Arduino_ESP32_OTA::Error::None)
    {
      State = ERROR;
      return SLEEP_OKAY;
    }
    {
      int err = ota->startDownload(url);

      if(err < 0) {
        State = ERROR;
        return SLEEP_OKAY;
      }
    }
    State = DOWNLOADING;
    return SLEEP_OKAY;
  case DOWNLOADING:
    if(Wireless.State != Wireless::WIFI_CONNECTED) {
      ota->reset();
      State = IDLE;
      return SLEEP_OKAY;
    }
    if((downloadResult = ota->downloadPoll()) == 0) return SLEEP_OKAY;
    if(downloadResult < 0){
      State = ERROR;
      return SLEEP_OKAY;
    }
    State = UPDATE_READY;
    return SLEEP_OKAY;
  }
}

void Ota::DownloadUpdateListTask(void* arg)
{
  Ota* ota = (Ota*)arg;
  client.setInsecure();
  HttpClient https(client, Config.GetString("Ota", "ServerName"), HttpClient::kHttpsPort);
  String path = Config.GetString("Ota", "Path");
  path += "VERSION.CFG";
  auto error = https.get(path);
  if (error != 0) {
    Serial.printf("HTTP GET failed, error: %d\n", error);
    ota->State = ERROR;
    return;
  }
  ArduinoJson::JsonDocument versions;
  String payload = https.responseBody(); // Get the response as a string
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(versions, payload);

  if (jsonError) {
    Serial.printf("JSON deserialization failed: %s\n", jsonError.c_str());
    ota->State = ERROR;
    return;
  }
  ArduinoJson::JsonObject version = versions[ota->GetVersion()];
  if(version.isNull()) {
    ota->State = ERROR;
  }
  const char* updateTarget = version["UpdatesTo"];
  if(updateTarget == NULL){
    ota->State = NO_UPDATE;
  }
  strcpy(url, Config.GetString("Ota", "Url"));
  strcat(url, updateTarget);
  strcat(url, ".ota");
  ota->State = UPDATE_AVAILABLE;
}
