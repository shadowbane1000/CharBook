#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "module.h"

class WebServer : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
};

extern WebServer WebServer;

#endif // __WEBSERVER_H__
