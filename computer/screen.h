#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <ArduinoJson.h>
#include "src/module.h"
#include "screentemplate.h"

class Screen : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
  bool Push(const char* filename);
  bool Pop();

private:
  int screenIndex;
  const int MAX_SCREEN_DEPTH = 5;
  ArduinoJson::JsonDocument data;
  char* stack;
  static ScreenTemplate* const templates[];
  ScreenTemplate* currentTemplate;

  void Render();
  void FindTemplate();
};

#endif // __SCREEN_H__
