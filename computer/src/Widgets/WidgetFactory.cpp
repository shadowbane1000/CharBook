#include "Widgets/WidgetFactory.h"

// Include all widget headers that the factory can create
#include "Widgets/BatteryWidget.h"
#include "Widgets/ContainerWidget.h"
#include "Widgets/ImageWidget.h"
#include "Widgets/LabelWidget.h"
#include "Widgets/PopScreenWidget.h"
#include "Widgets/SettingsWidget.h"
#include "Widgets/TileHolderWidget.h"
#include "Widgets/TileWidget.h"
#include "Widgets/TopBarWidget.h"
#include "Widgets/WirelessWidget.h"

Widget *WidgetFactory::createWidget(const JsonObject &config)
{
  if (!config["type"].is<String>())
  {
    Serial.println("Widget config missing 'type' field");
    return nullptr;
  }

  String type = config["type"].as<String>();

  if (type.equalsIgnoreCase("Battery"))
  {
    return new BatteryWidget(config);
  }
  else if (type.equalsIgnoreCase("Container"))
  {
    return new ContainerWidget(config);
  }
  else if (type.equalsIgnoreCase("Image"))
  {
    return new ImageWidget(config);
  }
  else if (type.equalsIgnoreCase("Label"))
  {
    return new LabelWidget(config);
  }
  else if (type.equalsIgnoreCase("PopScreen"))
  {
    return new PopScreenWidget(config);
  }
  else if (type.equalsIgnoreCase("Settings"))
  {
    return new SettingsWidget(config);
  }
  else if (type.equalsIgnoreCase("TileHolder"))
  {
    return new TileHolderWidget(config);
  }
  else if (type.equalsIgnoreCase("TopBar"))
  {
    return new TopBarWidget(config);
  }
  else if (type.equalsIgnoreCase("Wireless"))
  {
    return new WirelessWidget(config);
  }
  else
  {
    Serial.printf("Unknown widget type: %s\r\n", type.c_str());
    return nullptr;
  }
}
