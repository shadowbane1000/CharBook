#include <Update.h>
#include <ArduinoJson.hpp>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ESP32-targz.h>
#include <SimpleFTPServer.h>

#include "ScreenTemplates/ScreenTemplate_Settings.h"
#include "Widgets/TopBarWidget.h"
#include "Widgets/ImageWidget.h"

#include "display.h"
#include "screen.h"
#include "wireless.h"
#include "config.h"
#include "version.h"
#include "filesystem.h"

AsyncWebServer* ScreenTemplate_Settings::webServer;

ScreenTemplate_Settings::PrivateWifiWidget::PrivateWifiWidget(uint8_t* imageData, uint16_t imageSize)
  : TileWidget(1, 3) // 1x3 tile
{
  m_imageWidget = new ImageWidget(imageData, imageSize, 0, 0);
  Children.push_back(m_imageWidget);
}

ScreenTemplate_Settings::PublicWifiWidget::PublicWifiWidget(uint8_t* imageData, uint16_t imageSize)
  : TileWidget(1, 3) // 1x3 tile
{
  m_imageWidget = new ImageWidget(imageData, imageSize, 0, 0);
  Children.push_back(m_imageWidget);
}

ScreenTemplate_Settings::UpdateWidget::UpdateWidget(uint8_t* imageData, uint16_t imageSize)
  : TileWidget(1, 3) // 1x3 tile
{
  m_imageWidget = new ImageWidget(imageData, imageSize, 0, 0);
  Children.push_back(m_imageWidget);
}

void ScreenTemplate_Settings::TouchToExit()
{
  int x,y;
  while(!Screen.GetTouch(&x, &y)) delay(100);
  // redraw what should be there.
  Display.Flush();
}

void ScreenTemplate_Settings::handleNotFound(AsyncWebServerRequest* request) {
  String message = "File Not Found\n\n";
  message += "URL: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}


FtpServer ftpServer;

void ScreenTemplate_Settings::RunWebServer(std::vector<String>& ScannedNetworks, std::function<IdleCallback> idleCallback)
{
  int x,y;
  webServer = new AsyncWebServer(80);
  webServer->begin();

  ftpServer.begin();
  ftpServer.credentials("charbook", "charbook");

  webServer->onNotFound(handleNotFound);
  //webServer->serveStatic("/index.html", SD, "/" VERSION "/WWW/INDEX.HTM");
  webServer->on("/RESET_MAIN", [](AsyncWebServerRequest* request){
    // TODO: read CURRENT.CFG, remove screen stack, save CURRENT.CFG
    // or should I have the UI do it?
    request->send(200);
    ESP.restart();
  });
  webServer->on("/RESET_FACTORY", [](AsyncWebServerRequest* request){
    FileSystem.Remove("CURRENT.CFG");
    request->send(200);
    ESP.restart();
  });
  webServer->on("/WIFI", [ScannedNetworks](AsyncWebServerRequest* request){
    // TODO: perform WIFI scan
    String foundNetworks;
    for(auto net : ScannedNetworks)
    {
      if(Config.GetString("WiFi", net.c_str()) != NULL){
        Serial.printf("Known network found %s\r\n", net.c_str());
        continue;
      } 
      if(foundNetworks.length() != 0) foundNetworks += ",";
      foundNetworks += "\""+net+"\"";
      Serial.printf("WiFi scan found unknown network %s\r\n", net.c_str());
    }
    foundNetworks = "[" + foundNetworks + "]";
    request->send(200, "text/json", foundNetworks);
  });

  while(!Screen.GetTouch(&x, &y)){
    idleCallback();
    ftpServer.handleFTP();
    delay(1);
  }
  ftpServer.end();
  webServer->end();
  Wireless.WifiDisconnect();
  Display.Flush();
  delete webServer;
  webServer = NULL;
}

bool ScreenTemplate_Settings::PrivateWifiWidget::HandleTouch(uint16_t x, uint16_t y)
{
  Display.ClearImmediate(0x0F);
  Display.CursorX = 0;
  Display.CursorY = 40;
  Display.DrawTextImmediate("Scanning for etworks...\n");
  std::vector<String> ScannedNetworks;
  Wireless.WifiScan(ScannedNetworks);
  Display.DrawTextImmediate("Creating Private Wifi Network...\n");
  // TODO: add DNS entry for captured portal
  Wireless.WifiDisconnect();
  if(Wireless.WifiPrivate()){
    char buffer[200];
    sprintf(buffer, "WiFi network %s created.\nPassword is \"%s\"\nAccess it now to configure this CharBook.\nTouch To close network and exit\n");
    Display.DrawTextImmediate(buffer);
    RunWebServer(ScannedNetworks, [](){ });
    ESP.restart();
  } else {
    ErrorAndTouch("Failed to start private wifi\n");
  }
  return true;
}

void ScreenTemplate_Settings::ErrorAndTouch(const char* message)
{
  Display.DrawTextImmediate(message);
  Display.DrawTextImmediate("Touch To Exit");
  TouchToExit();
  Wireless.WifiDisconnect();
}

bool ScreenTemplate_Settings::ConnectToWifi(std::vector<String>& scannedNetworks)
{
  Display.ClearImmediate(0x0F);
  Display.CursorX = 0;
  Display.CursorY = 40;
  Display.DrawTextImmediate("Scanning for Wifi Networks...\n");
  if(!Wireless.WifiScan(scannedNetworks)){
    ErrorAndTouch("Scanning failed\n");
    return false;
  }

  String foundNetwork;
  for(auto network : scannedNetworks)
  {
    if(Config.GetString("Wifi", network.c_str()) != NULL){
      foundNetwork = network;
      break;
    }
  }
  if(foundNetwork.isEmpty()){
    ErrorAndTouch("No known networks found.\nPlease use the private network to\nconfigure the password for another network\n");
    return false;
  }

  char buffer[200];
  sprintf(buffer, "Connecting to %s\n", foundNetwork.c_str());
  Display.DrawTextImmediate(buffer);
  if(!Wireless.WifiConnect(foundNetwork.c_str()))
  {
    ErrorAndTouch("Connection failed!\nPlease use the private network to\nconfigure the password for another network\n");
    return false;
  }
  return true;
}

bool ScreenTemplate_Settings::PublicWifiWidget::HandleTouch(uint16_t x, uint16_t y)
{
  std::vector<String> scannedNetworks;
  if(!ScreenTemplate_Settings::ConnectToWifi(scannedNetworks)) return true;

  String ipaddr = Wireless.GetIPAddress();
  char buffer[200];
  sprintf(buffer,"Connected\nGo to http://%s/ to configure\nTouch to exit", ipaddr.c_str());
  Display.DrawTextImmediate(buffer);
  RunWebServer(scannedNetworks, [](){});
  ESP.restart();
  return true;
}

bool ScreenTemplate_Settings::UpdateWidget::HandleTouch(uint16_t x, uint16_t y)
{
  std::vector<String> scannedNetworks;
  if(!ScreenTemplate_Settings::ConnectToWifi(scannedNetworks)) return true;

  char buffer[200];

  Display.DrawTextImmediate("Looking for update information\n");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  String url = Config.GetString("Ota", "Server");
  url += Config.GetString("Ota", "Path");
  https.begin(client, url + "VERSION.CFG");
  auto error = https.GET();
  if (error != 0) {
    sprintf(buffer, "HTTP GET failed, error: %d\n", error);
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return true;
  }
  ArduinoJson::JsonDocument versions;
  String payload = https.getString(); // Get the response as a string
  https.end();
  
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(versions, payload);

  if (jsonError) {
    sprintf(buffer, "JSON deserialization failed: %s\n", jsonError.c_str());
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return true;
  }
  ArduinoJson::JsonObject version = versions[VERSION];
  if(version.isNull()) {
    ScreenTemplate_Settings::ErrorAndTouch("Your version " VERSION " is not in the update doc\n");
    return true;
  }
  const char* updateTarget = version["UpdatesTo"];
  if(updateTarget == NULL){
    sprintf(buffer, "You are already on the latest version " VERSION "\nTouch To Exit");
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return true;
  }

  sprintf(buffer, "Updating from " VERSION " to %s\n", updateTarget);
  Display.DrawTextImmediate(buffer);

  url = Config.GetString("Ota", "Server");
  url += Config.GetString("Ota", "Path");
  https.begin(client, url + updateTarget + ".tgz");
  error = https.GET();
  if (error != 0) {
    Serial.printf("HTTP GET failed, error: %d\n", error);
    ScreenTemplate_Settings::ErrorAndTouch("Failed to download the flash file\n");
    return true;
  }

  Display.DrawTextImmediate("Downloading new flash\n");
  int bytes = 0;
  int lasterror = 0;
/*
  int percent = 0;
  int spacing = https.contentLength() / 10;
  int rootx = Display.CursorX;
  int rooty = Display.CursorY;
  String tgzFile = "/";
  tgzFile += updateTarget;
  tgzFile += ".tgz";
  if(SD.exists(tgzFile)) SD.remove(tgzFile);
  File file = SD.open(tgzFile, FILE_WRITE);
  while(!https.endOfBodyReached())
  {
    uint8_t buff[2048];
    int count = https.read(buff, 2048);
    if(count == -1){
      delay(100);
      continue;
    }
    if(bytes == 0){
      Serial.printf("First bytes %d  %d  %d\r\n", buff[0], buff[1], buff[2]);
    }
    file.write(buff, count);
    if(count < 0){
      if(lasterror == count){
        delay(100);
        continue;
      }
      lasterror = count;
      Serial.printf("Error %d\r\n", count);
      delay(100);
      continue;
    }
    if(count == 0) break;
    bytes += count;
    int curr = bytes / spacing;
    if(curr != percent) {
      Display.CursorY = rooty;
      Display.DrawTextImmediate("X");
      Display.CursorY = rooty;
      percent = curr;
    }
    Serial.printf("Byte %d of %d\r\n", bytes, https.contentLength());
    delay(1);
  }
  file.close();
  Display.DrawTextImmediate("\n");
  Display.CursorX = rootx;
  Display.DrawTextImmediate("Flash downloaded\n");
  https.stop();
  ApplyArchive(updateTarget);
*/
  return true;
}

void ScreenTemplate_Settings::ApplyArchive(const char* updateTarget)
{
  TaskHandle_t task;
  typedef struct {
    SemaphoreHandle_t semaphoreHandle;
    const char* updateTarget;
  } Args;
  Args args {
    .semaphoreHandle = xSemaphoreCreateBinary(),
    .updateTarget = updateTarget
  };
//  Serial.println("Taking semaphore");
//  xSemaphoreTake(args.semaphoreHandle, portMAX_DELAY);
  Serial.println("STarting task");
  xTaskCreate([](void* data){
    Serial.println("Within task");
    Display.DrawTextImmediate("Expanding Archive\n");
    Args* args = (Args*)data;
    String tgzFile = "/";
    tgzFile += args->updateTarget;
    tgzFile += ".tgz";
    char buffer[200];
    Serial.println("starting unpack");
    TarGzUnpacker *TARGZUnpacker = new TarGzUnpacker();
    TARGZUnpacker->haltOnError( true ); // stop on fail (manual restart/reset required)
    TARGZUnpacker->setTarVerify( true ); // true = enables health checks but slows down the overall process
    TARGZUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
    TARGZUnpacker->setGzProgressCallback( BaseUnpacker::defaultProgressCallback ); // targzNullProgressCallback or defaultProgressCallback
    TARGZUnpacker->setLoggerCallback( BaseUnpacker::targzPrintLoggerCallback  );    // gz log verbosity
    //TARGZUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
    TARGZUnpacker->setTarStatusProgressCallback( BaseUnpacker::defaultTarStatusProgressCallback ); // print the filenames as they're expanded
    TARGZUnpacker->setTarMessageCallback( BaseUnpacker::targzPrintLoggerCallback ); // tar log verbosity
  
    // or without intermediate file
    //if( !TARGZUnpacker->tarGzExpander(SD, tgzFile.c_str(), SD, "/", nullptr ) ) {
//      sprintf(buffer, "tarGzExpander failed with return code #%d\n", TARGZUnpacker->tarGzGetError() );
      //ErrorAndTouch(buffer);
      //xSemaphoreGive(args->semaphoreHandle);
      //vTaskDelete(NULL);
    //}
  
    Display.DrawTextImmediate("Updating flash\n");
    String flashFile = "/";
    flashFile += args->updateTarget;
    flashFile += "/Flash.gz";
    //if(!TARGZUnpacker->gzUpdater(SD, flashFile.c_str(), U_FLASH, true))
    //{
//      ScreenTemplate_Settings::ErrorAndTouch("Failed to update flash...\n");
      //xSemaphoreGive(args->semaphoreHandle);
      //vTaskDelete(NULL);  
    //}
  
    // reset to new version...
    Display.DrawTextImmediate("Resetting\n");
    ESP.restart();
    ScreenTemplate_Settings::ErrorAndTouch("Failed to reset after updating flash...\n");  
    xSemaphoreGive(args->semaphoreHandle);
    vTaskDelete(NULL);
  }, "GzUnpacker", 4000, (void*)&args, 0, &task);
  unsigned int ulNotifiedValue;
  Serial.println("Taking semaphore");
  xSemaphoreTake(args.semaphoreHandle, portMAX_DELAY);
  Serial.println("Deleting task");
  vTaskDelete(task);
}

void ScreenTemplate_Settings::Initialize(JsonDocument& screen)
{
  ScreenTemplate::Initialize(screen);

  tileHolder.SetX(0);
  tileHolder.SetY(80);
  tileHolder.SetWidth(Display.Width);
  tileHolder.SetHeight(Display.Height - 80);
  topBarWidget.SetX(0);
  topBarWidget.SetY(0);
  topBarWidget.SetWidth(Display.Width);
  topBarWidget.SetHeight(80);

  topBarWidget.ShowPopButton();
}
