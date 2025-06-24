#include "Widgets/LabelWidget.h"
#include "display.h"
#include <ArduinoJson.h> // Include for JsonObject
#include <Arduino.h>     // For Serial
#include "Widgets/TileHolderWidget.h" // Include for CELL_WIDTH, CELL_HEIGHT, SPACING
#include "json_helpers.h" // Include the new helper header
#include "lua_env.h"

// Constructor implementation from direct parameters
LabelWidget::LabelWidget(const String& text, int16_t x_pos, int16_t y_pos, int16_t width, int16_t height,
                         Display::Justification just, Display::Alignment align)
    : Text(text), Justification(just), Alignment(align)
{
    SetX(x_pos);
    SetY(y_pos);
    SetWidth(width);
    SetHeight(height);
    

    Serial.printf("LabelWidget created (direct): Text='%s', x=%d, y=%d, w=%d, h=%d, just=%d, align=%d\n",
                  Text.c_str(), GetX(), GetY(), GetWidth(), GetHeight(), Justification, Alignment);
}

// Constructor implementation from JsonObject
LabelWidget::LabelWidget(JsonObject config)
    : Text(config["text"] | "Label"),
      m_onTouchScript(config["onTouch"] | "") // Read onTouch script from JSON
{
    // Extract properties from JSON
    Text = config["text"] | "Label"; // Default text if missing

    // Parse position and dimensions using the helper function
    SetX(JsonHelpers::parseDimension(config["x"], TileHolderWidget::CELL_WIDTH, TileHolderWidget::SPACING));
    SetY(JsonHelpers::parseDimension(config["y"], TileHolderWidget::CELL_HEIGHT, TileHolderWidget::SPACING));
    SetWidth(JsonHelpers::parseDimension(config["width"], TileHolderWidget::CELL_WIDTH, TileHolderWidget::SPACING));
    SetHeight(JsonHelpers::parseDimension(config["height"], TileHolderWidget::CELL_HEIGHT, TileHolderWidget::SPACING));
    SetTransparent(config["transparent"] | false);

    // Default size if parsing resulted in 0
    if (GetWidth() == 0) SetWidth(50);
    if (GetHeight() == 0) SetHeight(10);

    // --- Justification ---
    const char* justificationStr = config["justification"] | "center"; // Default to center
    if (strcmp(justificationStr, "left") == 0) {
        Justification = Display::JUSTIFY_LEFT;
    } else if (strcmp(justificationStr, "right") == 0) {
        Justification = Display::JUSTIFY_RIGHT;
    } else { // Default to center
        Justification = Display::JUSTIFY_CENTER;
    }

    // --- Alignment ---
    const char* alignmentStr = config["alignment"] | "center"; // Default to center
    if (strcmp(alignmentStr, "top") == 0) {
        Alignment = Display::ALIGN_TOP;
    } else if (strcmp(alignmentStr, "bottom") == 0) {
        Alignment = Display::ALIGN_BOTTOM;
    } else { // Default to center
        Alignment = Display::ALIGN_CENTER;
    }

    Serial.printf("LabelWidget created: Text='%s', x=%d, y=%d, w=%d, h=%d, just=%d, align=%d\n",
                  Text.c_str(), GetX(), GetY(), GetWidth(), GetHeight(), Justification, Alignment);
}

void LabelWidget::Render(int16_t parentx, int16_t parenty)
{
  Serial.printf("LabelWidget::Render: Text='%s', x=%d, y=%d, w=%d, h=%d\n",
                  Text.c_str(), GetX(), GetY(), GetWidth(), GetHeight());
  // Set the position of the label based on its parent's position
  int16_t absoluteX = parentx + GetX();
  int16_t absoluteY = parenty + GetY();

  // Draw the text at the calculated position using configured justification/alignment
  Display.DrawText(Display::Region {
    .x = absoluteX,
    .y = absoluteY,
    .width = GetWidth(),
    .height = GetHeight()
  }, Justification, Alignment, Text, Transparent);
}

void LabelWidget::Update()
{
}

bool LabelWidget::HandleTouch(uint16_t x, uint16_t y)
{
    Serial.printf("LabelWidget::HandleTouch at (%d, %d)\n", x, y);
    if (m_onTouchScript.isEmpty()) return false; // Return false if script is missing or empty

    // Execute script and return its result
    LuaValueVariant result = LuaEnv.ExecuteScript(m_onTouchScript.c_str());
    if(result.type == LuaReturnType::ERROR) {
        Serial.printf("Error executing script: %s\n", result.error_message);
        return false;
    }
    if(result.type == LuaReturnType::BOOLEAN) {
        return result.value.b_val; // Return the boolean result of the script
    }
    return false;
}
