#include <Arduino.h>
#include <strings.h>
#include <TouchDrvGT911.hpp>  //Arduino IDE -> Library manager -> Install SensorLib v0.19 
#include <Wire.h>

#include "screen.h"
#include "filesystem.h"
#include "display.h"
#include "config.h"
#include "ota.h"
#include "wireless.h"
#include "batterymonitor.h"
#include "src/LilyGo-EPD47/src/utilities.h"

TouchDrvGT911 touch;
int touch_loop_interval;

class ScreenTemplate_Main : public ScreenTemplate
{
  WirelessButton wirelessButton;
  BatteryStatusIndicator batteryStatusIndicator;
  SettingsButton settingsButton;

public:
  ScreenTemplate_Main(){
    Children.push_back(&wirelessButton);
    Children.push_back(&batteryStatusIndicator);
    Children.push_back(&settingsButton);
  }
  virtual const char* Name(){ return "Main"; }
} ScreenTemplate_Main;

ScreenTemplate* const Screen::templates[] = {
  &ScreenTemplate_Main,
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
  pinMode(TOUCH_INT, OUTPUT);
  digitalWrite(TOUCH_INT, HIGH);

  // The touch reset pin uses hardware pull-up,
  // and the function of setting the I2C device address cannot be used.
  // Use scanning to obtain the touch device address.*/
  uint8_t touchAddress = 0;
  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) {
      touchAddress = 0x14;
  }
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) {
      touchAddress = 0x5D;
  }
  if (touchAddress == 0) {
      while (1) {
          Serial.println("Failed to find GT911 - check your wiring!");
          delay(1000);
      }
  }
  touch.setPins(-1, TOUCH_INT);
  if (!touch.begin(Wire, touchAddress, BOARD_SDA, BOARD_SCL )) {
      while (1) {
          Serial.println("Failed to find GT911 - check your wiring!");
          delay(1000);
      }
  }
  touch.setMaxCoordinates(Display.Width, Display.Height);
  touch.setSwapXY(true);
  touch.setMirrorXY(false, true);
  touch_loop_interval = millis() + 300;
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

  Config.SetArray("Screen", "Stack", stack, FileSystem.MAX_FILENAME_LENGTH, screenIndex);

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

  Config.SetArray("Screen", "Stack", stack, FileSystem.MAX_FILENAME_LENGTH, screenIndex);

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

  if (millis() > touch_loop_interval) {
    int16_t touch_x, touch_y;
    uint8_t touched = touch.getPoint(&touch_x, &touch_y);
    if (touched) {
      while(touch.getPoint(&touch_x, &touch_y));
      // got touched at x, y.  It's now been released
      currentTemplate->Touch(touch_x, touch_y);
      return SLEEP_OKAY;
    }
  }

  currentTemplate->Update();
  return SLEEP_OKAY;
}

void Screen::Render()
{
  currentTemplate->Render();
}

