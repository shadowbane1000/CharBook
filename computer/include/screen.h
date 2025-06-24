#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <ArduinoJson.h>
#include "module.h"
#include "screentemplates/screentemplate.h"

class Screen : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
  bool Push(const char* filename);
  bool Pop();

  // useful for wizard like steps that block everything.
  bool GetTouch(int* x, int* y);

private:
  int screenIndex;
  const int MAX_SCREEN_DEPTH = 5;
  ArduinoJson::JsonDocument data;
  char* stack;
  static ScreenTemplate* const templates[];
  ScreenTemplate* currentTemplate;

  void Render();
  bool FindTemplate();
  bool FindScreen(const char* screenName, char* filename);
};

extern Screen Screen;

#endif // __SCREEN_H__
