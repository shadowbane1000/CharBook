#pragma once

#include <ArduinoJson.h>
#include "Widget.h" // Base widget class

class WidgetFactory {
public:
    static Widget* createWidget(const JsonObject& config);
};
