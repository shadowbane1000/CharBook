#include <Arduino.h>
#include <ArduinoJson.h>
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
#include "panic.h"
#include "lua_env.h" // Include LuaEnv header

// Include Lua headers needed for the wrapper functions
extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
  #include "luatemplate.h"
}

//#define IGNORE_TOUCH

TouchDrvGT911 touch;
int touch_loop_interval;

#define SCREEN_NAME_LENGTH (FileSystem.MAX_FILENAME_LENGTH*2+15)
#define MAX_SCREEN_PATH_LENGTH (FileSystem.MAX_FILENAME_LENGTH*2+10)

#include "ScreenTemplates/ScreenTemplate_Standard.h"
#include "ScreenTemplates/ScreenTemplate_Settings.h"

class ScreenTemplate_Standard ScreenTemplate_Standard;
class ScreenTemplate_Settings ScreenTemplate_Settings;

ScreenTemplate* const Screen::templates[] = {
  &ScreenTemplate_Standard,
  &ScreenTemplate_Settings,
  NULL
};

//#define TOUCH_INT 47

// --- Lua Wrapper Functions defined locally ---

static int lua_Screen_Push(lua_State *L) {
    // Get the Screen* pointer from the userdata (first argument)
    class Screen* screen = *(class Screen**)luaL_checkudata(L, 1, "Screen");
    luaL_argcheck(L, screen != NULL, 1, "'Screen' expected");

    // Get the screen name string (second argument)
    const char* screenName = luaL_checkstring(L, 2);

    // Call the actual method
    bool result = screen->Push(screenName);

    // Push the boolean result onto the Lua stack
    lua_pushboolean(L, result);

    // Return the number of return values (1 in this case)
    return 1;
}

static int lua_Screen_Pop(lua_State *L) {
    // Get the Screen* pointer from the userdata (first argument)
    class Screen* screen = *(class Screen**)luaL_checkudata(L, 1, "Screen");
    luaL_argcheck(L, screen != NULL, 1, "'Screen' expected");

    // Call the actual method
    bool result = screen->Pop();

    // Push the boolean result onto the Lua stack
    lua_pushboolean(L, result);

    // Return the number of return values (1 in this case)
    return 1;
}

void Screen::Initialize()
{
  stack = new char[MAX_SCREEN_DEPTH*SCREEN_NAME_LENGTH];
  screenIndex = 0;

  // Sleep wakeup must wait one second, otherwise the touch device cannot be addressed
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
    delay(1000);
  }
  
  Wire.begin(BOARD_SDA, BOARD_SCL);
  pinMode(TOUCH_INT, OUTPUT);
  digitalWrite(TOUCH_INT, HIGH);

  #ifndef IGNORE_TOUCH
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
  touch.setSwapXY(false);
  touch.setMirrorXY(false, false);
  touch_loop_interval = millis() + 300;
  #endif

  // --- Register Screen with Lua ---
  LuaEnv.RegisterClass("Screen");
  LuaEnv.RegisterMethod("Screen", "push", lua_Screen_Push);
  LuaEnv.RegisterMethod("Screen", "pop", lua_Screen_Pop);
  LuaEnv.RegisterObject("Screen", "Screen", this);
  Serial.println("Screen object registered with Lua.");
  // ------------------------------
}

bool Screen::FindScreen(const char* screenName, char* filename)
{
  strcpy(filename, "/");
  strcat(filename, screenName);
  strcat(filename, ".SCN");
  if(FileSystem.Exists(filename)) return true;
  return false;
}

bool Screen::Pop()
{
  DumpStack();
  if(screenIndex < 2) return false;
  if(!FileSystem.Available()) return false;

  screenIndex--;

  char filePath[MAX_SCREEN_PATH_LENGTH];
  char* screenName = &stack[(screenIndex)*SCREEN_NAME_LENGTH];
  Serial.printf("Popping screen %s\r\n", screenName);
  screenName = &stack[(screenIndex-1)*SCREEN_NAME_LENGTH];
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

  ArduinoJson::JsonDocument* newData = new ArduinoJson::JsonDocument();
  if(!FileSystem.LoadJson(filePath, newData)) return false;

  DeinitializeTemplate();
  delete data;
  data = newData;

  ExpandTemplateFiles();
  FindTemplate();
  InitializeTemplate();
  Render();

  //Config.SetArray("Screen", "Stack", stack, SCREEN_NAME_LENGTH, screenIndex);
  DumpStack();

  return true;
}

bool Screen::Push(const char* screenName)
{
  DumpStack();
  Serial.printf("Pushing screen %s\r\n", screenName);
  if(screenIndex == MAX_SCREEN_DEPTH) return false;
  if(!FileSystem.Available()) return false;

  char filePath[MAX_SCREEN_PATH_LENGTH];
  if(!FindScreen(screenName, filePath)){
    Serial.printf("Can't find screen %s\r\n", screenName);
    return false;
  }

  ArduinoJson::JsonDocument* newData = new ArduinoJson::JsonDocument();  
  if(!FileSystem.LoadJson(filePath, newData)){
    Serial.printf("Failed to load screen file %s\r\n", filePath);
    return false;
  } 

  // success
  DeinitializeTemplate();
  String name = screenName;
  delete data;
  data = newData;
  strcpy(&stack[screenIndex*SCREEN_NAME_LENGTH], name.c_str());
  screenIndex++;

  if(!ExpandTemplateFiles()){
    Pop();
    return false;
  }
  if(!FindTemplate()){
    Pop();
    return false;
  }
  InitializeTemplate();
  Render();

  DumpStack();
  //Config.SetArray("Screen", "Stack", stack, SCREEN_NAME_LENGTH, screenIndex);

  return true;
}

// --- Helper method to transform TPL content to Lua script ---
String Screen::TransformTplToLua(const char* tpl) 
{
  LuaEnv.RegisterJsonAsGlobalTable("screen", data);
  return luaTemplateToLua(tpl);
}

bool Screen::ExpandTemplateFiles()
{
  String templateFileName = (*data)["template"].as<String>(); // e.g., "MyTemplate.tpl" or just "MyTemplate" if .tpl is implicit
  // Assuming templateFileName is the actual name of the TPL file.
  // If it's just a base name, you might need to append ".tpl" here.

  if(!FileSystem.Exists(templateFileName.c_str())) {
    // This could mean it's a built-in template name not corresponding to a .TPL file,
    // or the .SCN file doesn't use a separate .TPL file.
    return true; 
  }
  
  uint32_t size;
  char* tplContent = (char*)FileSystem.Load(templateFileName.c_str(), &size);
  if (tplContent == NULL) {
    Serial.printf("Failed to load TPL file content: %s\r\n", templateFileName.c_str());
    // Depending on desired behavior, this could be a fatal error for this screen.
    // For now, let's say it's not fatal, but the template won't be expanded.
    return true; // Or false if this is critical
  }

  String luaScript = TransformTplToLua(tplContent);
  free(tplContent);

  Serial.println("--- Generated Lua Script from TPL ---");
  Serial.println(luaScript);
  Serial.println("------------------------------------");

  // TODO: Execute the Lua script and get the result
  LuaValueVariant result = LuaEnv.ExecuteScript(luaScript.c_str());
  if(result.type == LuaReturnType::ERROR) {
    Serial.printf("Error executing Lua script: %s\r\n", result.error_message);
    return false;
  }
  if(result.type != LuaReturnType::STRING) {
    Serial.printf("Lua script did not return a string, but type %d\r\n", (int)result.type);
    return false;
  }
  Serial.println("--- Template result from Lua script ---\r\n");
  Serial.println(result.value.s_val);
  Serial.println("------------------------------------");
  ArduinoJson::DeserializationError error = ArduinoJson::deserializeJson(*data, result.value.s_val);
  if(error){
    Serial.printf("Failed to deserialize template result (%s)\r\n", error.c_str());
    Serial.printf("Template result: %s\r\n", result.value.s_val);
    return false;
  }
  return true;
}

bool Screen::FindTemplate()
{
  String templateName = (*data)["template"].as<String>();
  Serial.printf("Searching for template %s\r\n", templateName.c_str());
  int i = 0;
  while(templates[i] != NULL){
    if(strcasecmp(templates[i]->Name(), templateName.c_str()) == 0){
      currentTemplate = templates[i];
      return true;
    }
    i++;
  }
  Serial.printf("Template not found: %s\r\n", (*data)["template"]);
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
    static int i = 0;
    i++;
    if(i > 2) panic("Can't load SYSTEMS screen");
    Push("SYSTEMS");
    return SLEEP_OKAY;
  }
  if(currentTemplate == NULL) return SLEEP_OKAY;

  #ifndef IGNORE_TOUCH
  if (millis() > touch_loop_interval) {
    int16_t touch_x, touch_y;
    uint8_t touched = touch.getPoint(&touch_x, &touch_y);
    if (touched) {
      do
      {
        delay(50);
        touched = touch.getPoint(&touch_x, &touch_y);
      } while(touched);
      // got touched at x, y.  It's now been released
      Serial.printf("Touch at %d,%d\r\n", touch_x, touch_y);
      currentTemplate->Touch(touch_x, touch_y);
      touch_loop_interval = millis() + 300;
      return SLEEP_OKAY;
    }
  }
  #endif

  currentTemplate->Update();
  return SLEEP_OKAY;
}

void Screen::InitializeTemplate()
{
  if(currentTemplate == NULL) return;
  currentTemplate->Initialize(*data);
}

void Screen::DeinitializeTemplate()
{
  if(currentTemplate == NULL) return;
  currentTemplate->Deinitialize();
}

void Screen::DumpStack()
{
  Serial.println("Screen Stack:");
  for(int i = 0; i < screenIndex; i++){
    Serial.printf("  %d: %s\r\n", i, &stack[i*SCREEN_NAME_LENGTH]);
  }
}

void Screen::Render()
{
  Display.Clear(0x0F);
  if(currentTemplate == NULL) return;
  Display.OffsetX = 0;
  Display.OffsetY = 0;
  currentTemplate->Render(0,0);
  Serial.printf("HeapSize: %d\r\nFreeHeap: %d\r\nMaxAlloc: %d\r\nTotal PSRAM: %d\r\nFree PSRAM: %d\r\n",
    ESP.getHeapSize(),
    ESP.getFreeHeap(),
    ESP.getMaxAllocHeap(),
    ESP.getPsramSize(),
    ESP.getFreePsram());
}

