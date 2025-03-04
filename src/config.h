#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "service.h"

class Config : public Module
{
public:
  enum State
  {

    STARTUP = 0,
    FILE_FOUND = 1,
    ERROR = 2,
    SETTINGS_LOADED = 3,
  } State;

  void Initialize();
  virtual Module::UpdateResponse Update();

  const char* GetString(const char* section, const char* key);
private:
  bool cardInserted;
};

extern Config Config;

#endif // __CONFIG_H__