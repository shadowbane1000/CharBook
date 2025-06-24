#include <Update.h>
#include <ArduinoJson.hpp>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <EspFileManager.h>
#include <DNSServer.h>
#include <ESP-FTP-Server-Lib.h>

#include "screentemplates/screentemplate_settings.h"

#include "display.h"
#include "screen.h"
#include "wireless.h"
#include "config.h"
#include "version.h"

AsyncWebServer* ScreenTemplate_Settings::webServer;

ScreenTemplate_Settings::PrivateWifiWidget::PrivateWifiWidget() : CardWidget(PRIVWIFI_png, PRIVWIFI_png_len)
{}

ScreenTemplate_Settings::UpdateWidget::UpdateWidget() : CardWidget(UPDATE_png, UPDATE_png_len)
{}

ScreenTemplate_Settings::PublicWifiWidget::PublicWifiWidget() : CardWidget(PUBWIFI_png, PUBWIFI_png_len)
{}

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

void ScreenTemplate_Settings::RunWebServer(std::vector<String>& ScannedNetworks, std::function<IdleCallback> idleCallback)
{
  int x,y;
  FTPServer ftpServer;
  webServer = new AsyncWebServer(80);
  webServer->begin();
  ftpServer.addUser("User", "Password");
  ftpServer.addFilesystem("SD", &SD);
  ftpServer.begin();

  webServer->onNotFound(handleNotFound);
  webServer->serveStatic("/index.html", SD, "/www/index.html");
  webServer->on("/RESET_MAIN", [](AsyncWebServerRequest* request){
    // TODO: read CURRENT.CFG, remove screen stack, save CURRENT.CFG
    // or should I have the UI do it?
    request->send(200);
    ESP.restart();
  });
  webServer->on("/RESET_FACTORY", [](AsyncWebServerRequest* request){
    SD.remove("/CURRENT.CFG");
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

  EspFileManager fileManager;
  fileManager.setFileSource(&SD);
  fileManager.setServer(webServer);

  while(!Screen.GetTouch(&x, &y)){
    idleCallback();
    ftpServer.handle();
    delay(1);
  }
  //ftpServer.end();
  webServer->end();
  Wireless.WifiDisconnect();
  Display.Flush();
  delete webServer;
  webServer = NULL;
}

void ScreenTemplate_Settings::PrivateWifiWidget::HandleTouch(uint16_t x, uint16_t y)
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
    DNSServer dnsServer;
    dnsServer.start(53, "*", WiFi.softAPIP());
    RunWebServer(ScannedNetworks, [&dnsServer](){ dnsServer.processNextRequest();});
    ESP.restart();
  } else {
    ErrorAndTouch("Failed to start private wifi\n");
  }
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

void ScreenTemplate_Settings::PublicWifiWidget::HandleTouch(uint16_t x, uint16_t y)
{
  std::vector<String> scannedNetworks;
  if(!ScreenTemplate_Settings::ConnectToWifi(scannedNetworks)) return;

  String ipaddr = Wireless.GetIPAddress();
  char buffer[200];
  sprintf(buffer,"Connected\nGo to http://%s/ to configure\nTouch to exit", ipaddr.c_str());
  Display.DrawTextImmediate(buffer);
  RunWebServer(scannedNetworks, [](){});
  ESP.restart();
  return;
}

void ScreenTemplate_Settings::UpdateWidget::HandleTouch(uint16_t x, uint16_t y)
{
  std::vector<String> scannedNetworks;
  if(!ScreenTemplate_Settings::ConnectToWifi(scannedNetworks)) return;

  char buffer[200];

  Display.DrawTextImmediate("Looking for update information\n");
  WiFiClientSecure client;
  client.setInsecure();
  HttpClient https(client, Config.GetString("Ota", "Server"), HttpClient::kHttpsPort);
  String path = Config.GetString("Ota", "Path");
  path += "VERSION.CFG";
  Serial.println(Config.GetString("Ota", "Server"));
  Serial.println(path);
  auto error = https.get(path);
  if (error != 0) {
    sprintf(buffer, "HTTP GET failed, error: %d\n", error);
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return;
  }
  ArduinoJson::JsonDocument versions;
  String payload = https.responseBody(); // Get the response as a string
  https.stop();
  
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(versions, payload);

  if (jsonError) {
    sprintf(buffer, "JSON deserialization failed: %s\n", jsonError.c_str());
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return;
  }
  ArduinoJson::JsonObject version = versions[VERSION];
  if(version.isNull()) {
    ScreenTemplate_Settings::ErrorAndTouch("Your version " VERSION " is not in the update doc\n");
    return;
  }
  const char* updateTarget = version["UpdatesTo"];
  if(updateTarget == NULL){
    sprintf(buffer, "You are already on the latest version " VERSION "\nTouch To Exit");
    ScreenTemplate_Settings::ErrorAndTouch(buffer);
    return;
  }

  sprintf(buffer, "Updating from " VERSION " to %s\n", updateTarget);
  Display.DrawTextImmediate(buffer);

  path = Config.GetString("Ota", "Path");
  path += updateTarget;
  path += ".ota";
  Serial.println(Config.GetString("Ota", "Server"));
  Serial.println(path);
  error = https.get(path);
  if (error != 0) {
    Serial.printf("HTTP GET failed, error: %d\n", error);
    ScreenTemplate_Settings::ErrorAndTouch("Failed to download the flash file\n");
    return;
  }

  Display.DrawTextImmediate("Downloading new flash\n");
  int bytes = 0;
  int lasterror = 0;
  auto statusCode = https.responseStatusCode();
  if(statusCode == HTTP_ERROR_TIMED_OUT){
    ScreenTemplate_Settings::ErrorAndTouch("Timeout waiting to get flash from server\n");
    return;
  }
  while(!https.endOfHeadersReached())
  {
    int c = https.readHeader();
    if(c == '\n')
      delay(100);
  }

  UpdateClass update;
  //ota.setCACert(root_ca);
  if(update.begin(https.contentLength()) < 0)
  {
    ScreenTemplate_Settings::ErrorAndTouch("Failed to begin update\nTouch To Exit");
    return;
  }

  int percent = 0;
  int spacing = https.contentLength() / 10;
  int rootx = Display.CursorX;
  int rooty = Display.CursorY;
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
    update.write(buff, count);
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
  Display.DrawTextImmediate("\n");
  Display.CursorX = rootx;
  Display.DrawTextImmediate("Flash downloaded\n");
  https.stop();
  Display.DrawTextImmediate("Updating flash...\n");
  if(!update.end())
  {
    char buff[128];
    sprintf(buff, "Failed to update flash %s\n", update.errorString());
    ScreenTemplate_Settings::ErrorAndTouch(buff);
    return;
  }
  // reset to new version...
  Display.DrawTextImmediate("Resetting\n");
  ESP.restart();
  ScreenTemplate_Settings::ErrorAndTouch("Failed to reset after flash...\n");
  return;
}

