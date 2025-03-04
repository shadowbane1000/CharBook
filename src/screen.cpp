#include <Arduino.h>
#include <strings.h>

#include "screen.h"
#include "filesystem.h"
#include "display.h"

class ScreenTemplate_Main : public ScreenTemplate
{
public:
  virtual const char* Name(){ return "Main"; }
  virtual void Update(){ }
  virtual void Render(){ }
} ScreenTemplate_Main;

ScreenTemplate* const Screen::templates[] = {
  &ScreenTemplate_Main,
  NULL
};

void Screen::Initialize()
{
  stack = new char[MAX_SCREEN_DEPTH*FileSystem.MAX_FILENAME_LENGTH];
  screenIndex = 0;
}

bool Screen::Pop()
{
  if(screenIndex == 0) return false;
  if(!FileSystem.Available()) return false;

  char filename[FileSystem.MAX_FILENAME_LENGTH + 4];
  strcpy(filename, &stack[screenIndex*FileSystem.MAX_FILENAME_LENGTH]);
  strcat(filename, ".SCN");
  if(!FileSystem.Exists(filename)) return false;

  ArduinoJson::JsonDocument newData;  
  if(!FileSystem.LoadJson(filename, &newData)) return false;

  data = newData;
  screenIndex--;

  FindTemplate();
  Render();

  return true;
}

bool Screen::Push(const char* screenName)
{
  if(screenIndex == MAX_SCREEN_DEPTH) return false;
  if(!FileSystem.Available()) return false;

  char filename[FileSystem.MAX_FILENAME_LENGTH + 4];
  strcpy(filename, screenName);
  strcat(filename, ".SCN");
  if(!FileSystem.Exists(filename)) return false;

  ArduinoJson::JsonDocument newData;  
  if(!FileSystem.LoadJson(filename, &newData)) return false;

  // success
  data = newData;
  strcpy(&stack[screenIndex*FileSystem.MAX_FILENAME_LENGTH], filename);
  screenIndex++;

  FindTemplate();
  Render();

  return true;
}

void Screen::FindTemplate()
{
  ScreenTemplate* searchTemplate = templates[0];
  while(searchTemplate != NULL){
    if(strcasecmp(searchTemplate->Name(), data["template"]) == 0){
      currentTemplate = searchTemplate;
      return;
    }
    while(1); // not a valid template...
  }
}

Module::UpdateResponse Screen::Update()
{
  if(screenIndex == 0){
    Push("MAIN");
    return SLEEP_OKAY;
  }
  currentTemplate->Update();
  return SLEEP_OKAY;
}

void Screen::Render()
{
  currentTemplate->Render();
}

