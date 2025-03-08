#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "src/module.h"
#include "screentemplate.h"

class Config : public Module
{
  int saveDelay;
  const int UPDATES_BEFORE_FLUSH = 2;
public:
  enum State
  {

    STARTUP = 0,
    FILE_FOUND = 1,
    ERROR = 2,
    SETTINGS_LOADED = 3,
  } State;

  Config() {}
  void Initialize();
  virtual Module::UpdateResponse Update();

  const char* GetString(const char* section, const char* key);

  void SetArray(const char* section, const char* key, char* values, uint16_t valueLength, uint16_t valueCount);

private:
  bool cardInserted;
};

class SettingsButton : public FixedSizeWidget
{
public:
  SettingsButton(){
    x = Display.Width - 80*2;
    y = 0;
  }
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
};

extern Config Config;

#endif // __CONFIG_H__