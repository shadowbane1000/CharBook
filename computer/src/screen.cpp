#include <Arduino.h>
#include <strings.h>
#include <TouchDrvGT911.hpp>  //Arduino IDE -> Library manager -> Install SensorLib v0.19 
#include <Wire.h>

#include "screen.h"
#include "filesystem.h"
#include "display.h"
#include "config.h"
#include "wireless.h"
#include "batterymonitor.h"
#include "pinout.h"
#include "widgets/cardwidget.h"

TouchDrvGT911 touch;
int touch_loop_interval;

#define MAX_SCREEN_PATH_LENGTH (FileSystem.MAX_FILENAME_LENGTH*2+10)

#include "screentemplates/screentemplate_main.h"
#include "screentemplates/screentemplate_settings.h"

ScreenTemplate_Main ScreenTemplate_Main;
ScreenTemplate_Settings ScreenTemplate_Settings;

ScreenTemplate* const Screen::templates[] = {
  &ScreenTemplate_Main,
  &ScreenTemplate_Settings,
  NULL
};

//#define TOUCH_INT 47

void Screen::Initialize()
{
  stack = new char[MAX_SCREEN_DEPTH*FileSystem.MAX_FILENAME_LENGTH];
  screenIndex = 0;

  // Sleep wakeup must wait one second, otherwise the touch device cannot be addressed
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
    delay(1000);
  }
  Wire.begin(BOARD_SDA, BOARD_SCL);
  pinMode(TOUCH_INT, OUTPUT);
  digitalWrite(TOUCH_INT, HIGH);

  // The touch reset pin uses hardware pull-up,
  // and the function of setting the I2C device address cannot be used.
  // Use scanning to obtain the touch device address.*/
  uint8_t touchAddress = 0;
  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) {
    Serial.println("Touch screen found at 0x14");
    touchAddress = 0x14;
  }
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) {
    Serial.println("Touch screen found at 0x5D");
    touchAddress = 0x5D;
  }
  if (touchAddress == 0) {
      while (1) {
          Serial.println("Failed to find GT911 - check your wiring! (A)");
          delay(1000);
      }
  }
  touch.setPins(-1, TOUCH_INT);
  if (!touch.begin(Wire, touchAddress, BOARD_SDA, BOARD_SCL )) {
      while (1) {
          Serial.println("Failed to find GT911 - check your wiring! (B)");
          delay(1000);
      }
  }
  touch.setMaxCoordinates(Display.Width, Display.Height);
  touch.setSwapXY(true);
  touch.setMirrorXY(false, true);
  touch_loop_interval = millis() + 300;
}

bool Screen::FindScreen(const char* screenName, char* filename)
{
  strcpy(filename, "/COMMON/");
  strcat(filename, screenName);
  strcat(filename, ".SCN");
  if(FileSystem.Exists(filename)) return true;
  return false;
}

bool Screen::Pop()
{
  if(screenIndex < 2) return false;
  if(!FileSystem.Available()) return false;

  screenIndex--;

  char filePath[MAX_SCREEN_PATH_LENGTH];
  char* screenName = &stack[(screenIndex)*FileSystem.MAX_FILENAME_LENGTH];
  Serial.printf("Popping screen %s\r\n", screenName);
  screenName = &stack[(screenIndex-1)*FileSystem.MAX_FILENAME_LENGTH];
  if(!FindScreen(screenName, filePath))
  {
    Serial.printf("Can't find previous screen %s\r\n", screenName);
    screenIndex++;
    return false;
  }
  else
  {
    Serial.printf("Loading screen %s\r\n", filePath); 
  }

  ArduinoJson::JsonDocument newData;  
  if(!FileSystem.LoadJson(filePath, &newData)) return false;

  data = newData;

  FindTemplate();
  Render();

  //Config.SetArray("Screen", "Stack", stack, FileSystem.MAX_FILENAME_LENGTH, screenIndex);

  return true;
}

bool Screen::Push(const char* screenName)
{
  Serial.printf("Pushing screen %s\r\n", screenName);
  if(screenIndex == MAX_SCREEN_DEPTH) return false;
  if(!FileSystem.Available()) return false;

  char filePath[MAX_SCREEN_PATH_LENGTH];
  if(!FindScreen(screenName, filePath)){
    Serial.printf("Can't find screen %s\r\n", screenName);
    return false;
  }

  ArduinoJson::JsonDocument newData;  
  if(!FileSystem.LoadJson(filePath, &newData)){
    Serial.printf("Failed to load screen file %s\r\n", filePath);
    return false;
  } 

  // success
  data = newData;
  Serial.printf("new Screen template: %s\r\n", newData["template"].as<String>().c_str());
  strcpy(&stack[screenIndex*FileSystem.MAX_FILENAME_LENGTH], screenName);
  screenIndex++;

  if(!FindTemplate()){
    Pop();
    return false;
  }
  Render();

  //Config.SetArray("Screen", "Stack", stack, FileSystem.MAX_FILENAME_LENGTH, screenIndex);

  return true;
}

bool Screen::FindTemplate()
{
  String templateName = data["template"].as<String>();
  Serial.printf("Searching for template %s\r\n", templateName.c_str());
  int i = 0;
  while(templates[i] != NULL){
    if(strcasecmp(templates[i]->Name(), templateName.c_str()) == 0){
      currentTemplate = templates[i];
      return true;
    }
    i++;
  }
  Serial.printf("Template not found: %s\r\n", data["template"]);
  return false;
}

bool Screen::GetTouch(int* x, int* y)
{
  int16_t touch_x, touch_y;
  uint8_t touched = touch.getPoint(&touch_x, &touch_y);
  if (touched) {
    do{
      delay(50);
      touched = touch.getPoint(&touch_x, &touch_y);
    }while(touched);
    return true;
  }
  return false;
}

Module::UpdateResponse Screen::Update()
{
  if(screenIndex == 0){
    Push("MAIN");
    return SLEEP_OKAY;
  }

  if (millis() > touch_loop_interval) {
    int16_t touch_x, touch_y;
    uint8_t touched = touch.getPoint(&touch_x, &touch_y);
    if (touched) {
      do{
        delay(50);
        touched = touch.getPoint(&touch_x, &touch_y);
      }while(touched);
      // got touched at x, y.  It's now been released
      Serial.printf("Touch at %d,%d\r\n", touch_x, touch_y);
      currentTemplate->Touch(touch_x, touch_y);
      touch_loop_interval = millis() + 300;
      return SLEEP_OKAY;
    }
  }

  currentTemplate->Update();
  return SLEEP_OKAY;
}

void Screen::Render()
{
  Display.Clear(0x0F);
  currentTemplate->Render();
}

