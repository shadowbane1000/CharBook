#include <ArduinoJson.h>
#include <algorithm>

#include "ScreenTemplates/ScreenTemplate_Standard.h"
#include "screen.h"
#include "Widgets/TopBarWidget.h"

void ScreenTemplate_Standard::Deinitialize()
{
  auto it = std::find(Children.begin(), Children.end(), tileHolder);
  if (it != Children.end()) {
    Children.erase(it);
  }
  delete tileHolder;
  tileHolder = nullptr;
  topBarWidget.HidePopButton();
}

void ScreenTemplate_Standard::Initialize(ArduinoJson::JsonDocument& screen)
{
  ScreenTemplate::Initialize(screen);
  tileHolder = new TileHolderWidget(screen["tileholder"].as<JsonObject>());
  tileHolder->SetX(0);
  tileHolder->SetY(80);
  tileHolder->SetWidth(Display.Width);
  tileHolder->SetHeight(Display.Height - 80);
  topBarWidget.SetX(0);
  topBarWidget.SetY(0);
  topBarWidget.SetWidth(Display.Width);
  topBarWidget.SetHeight(80);
  Children.push_back(tileHolder);
  if(!Screen.TopOfStack()){
    topBarWidget.ShowPopButton();
  } else {
    topBarWidget.HidePopButton();
  }
}

ScreenTemplate_Standard::ScreenTemplate_Standard() :
  ScreenTemplate(),
  tileHolder(nullptr)
{
  Children.push_back(&topBarWidget);
}
