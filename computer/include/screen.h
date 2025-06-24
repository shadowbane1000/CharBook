#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <ArduinoJson.h>
#include "module.h"
#include "ScreenTemplates/ScreenTemplate.h"

class Screen : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
  bool Push(const char* filename);
  bool Pop();
  bool TopOfStack() { return screenIndex < 2; }

  // useful for wizard like steps that block everything.
  bool GetTouch(int* x, int* y);

private:
  int screenIndex;
  const int MAX_SCREEN_DEPTH = 5;
  ArduinoJson::JsonDocument* data;
  char* stack;
  static ScreenTemplate* const templates[];
  ScreenTemplate* currentTemplate;

  void DumpStack();
  void Render();
  bool ExpandTemplateFiles();
  bool FindTemplate();
  void InitializeTemplate();
  void DeinitializeTemplate();
  bool FindScreen(const char* screenName, char* filename);
  String TransformTplToLua(const char* tpl);
};

extern class Screen Screen;

#endif // __SCREEN_H__
