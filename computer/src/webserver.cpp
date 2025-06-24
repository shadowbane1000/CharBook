#include <ESPAsyncWebServer.h>
#include <SdFat.h>

#include "webserver.h"
#include "../filesystem.h"

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
  File32* file = (File32*)request->_tempObject;
  if(index == 0)
  {
    request->_tempObject = (void*)new File32();
    file = (File32*)request->_tempObject;
    if(!file->open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC)){
      delete file;
      request->_tempObject = 0;
      return;
    }
  }
  if(file == NULL) return;
  file->write(data, len);
  
  if(final){
    file->close();
  }
}

void WebServer::Initialize()
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
  
  server.begin();
}

Module::UpdateResponse WebServer::Update()
{
}

