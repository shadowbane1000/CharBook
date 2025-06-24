#include <ESPAsyncWebServer.h>
#include <SD.h>

#include "charbookwebserver.h"
#include "filesystem.h"
#include "wireless.h"

AsyncWebServer server(80);

void onNotFound(AsyncWebServerRequest *request)
{
  //Handle Unknown Request
  if (request->method() == HTTP_OPTIONS) 
  {
    request->send(200);
  }
  else
  {
    request->send(404);
  }
}

void onSdRead(AsyncWebServerRequest *request)
{
  String path = request->url();
  // TODO: parse path from url... we can't just use it directly.
  if(!FileSystem.Exists(path.c_str()))
  {
    request->send(404);
    return;
  }
  
  uint32_t size;
  uint8_t* buffer = FileSystem.Load(path.c_str(), &size);
  request->send(200, "application/octet-stream", buffer, size);
  free(buffer);
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  // TODO: change to use the filesystem class.... needs work.
  File* file = (File*)request->_tempObject;
  if(index == 0)
  {
    if(SD.exists(filename.c_str())) SD.remove(filename.c_str());
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if(!file)
    {
      request->_tempObject = 0;
      return;
    }
    
    request->_tempObject = (void*)new File();
    *(File*)request->_tempObject = file;
  }
  if(file == NULL) return;
  file->write(data, len);
  
  if(final){
    file->close();
  }
}

void CharBookWebServer::Initialize()
{
  server.on("/sd", HTTP_POST, [](AsyncWebServerRequest *request)
    {
      if(request->_tempObject == 0)
      {
        request->send(500);
      }
      else
      {
        request->send(204);
      }
    }, onUpload);

  server.on("/sd", HTTP_GET, onSdRead);
  
  server.onNotFound(onNotFound); 
}

Module::UpdateResponse CharBookWebServer::Update()
{
  if(Wireless.State == Wireless::WIFI_CONNECTED)
  {
    // if it changed to connected....
    // server.begin();
    // if it changed to disconnected, etc
    // server.end();
  }
  return SLEEP_OKAY;
}

