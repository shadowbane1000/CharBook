#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "module.h"

class CharBookWebServer : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
};

extern CharBookWebServer CharBookWebServer;

#endif // __WEBSERVER_H__
