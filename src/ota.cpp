#include <Arduino_ESP32_OTA.h>
#include <AsyncHTTPSRequest_Generic.h>
#include <ArduinoJson.hpp>

#include "ota.h"
#include "wireless.h"
#include "config.h"
#include "service.h"

static Arduino_ESP32_OTA* ota;

static AsyncHTTPSRequest request;
static char url[256];

const char* Ota::GetVersion()
{
  return "0.0.1";
}

void Ota::Initialize()
{
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
    request.debug();
    request.setSecure(false);
    if(!request.open("GET", url))
    {
      State = ERROR;
    }
    State = CONNECTING_TO_GET_VERSIONS_FILE;
    return SLEEP_OKAY;
  case CONNECTING_TO_GET_VERSIONS_FILE:
    if(request.readyState() == readyStateOpened)
    {
      request.send();
      State = GETTING_VERSIONS_FILE;      
    }
    return SLEEP_OKAY;
  case GETTING_VERSIONS_FILE:
    if(request.readyState() != readyStateDone) return SLEEP_OKAY;
    if(request.responseHTTPcode() != 200){
      State = ERROR;
      return SLEEP_OKAY;
    }
    {
      ArduinoJson::JsonDocument versions;
      ArduinoJson::DeserializationError error = ArduinoJson::deserializeJson(versions, request.responseLongText());
      if(error){
        State = ERROR;
        return SLEEP_OKAY;
      }
      ArduinoJson::JsonObject version = versions[GetVersion()];
      if(version.isNull()) {
        State = ERROR;
        return SLEEP_OKAY;
      }
      const char* updateTarget = version["UpdatesTo"];
      if(updateTarget == NULL){
        State = NO_UPDATE;
        return SLEEP_OKAY;
      }
      strcpy(url, Config.GetString("Ota", "Url"));
      strcat(url, updateTarget);
      strcat(url, ".ota");
      State = UPDATE_AVAILABLE;
      return SLEEP_OKAY;
    }
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

