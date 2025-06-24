#include "Widgets/TileWidget.h"
#include "Widgets/TileHolderWidget.h"
#include "Widgets/WidgetFactory.h"
#include "lua_env.h"
#include <ArduinoJson.h>
#include <Arduino.h>

TileWidget::TileWidget(int width_cells, int height_cells)
    : m_width_cells(width_cells), m_height_cells(height_cells)
{
    // Calculate initial pixel size based on cell size and spacing
    // The TileHolderWidget will set the final pixel position (X, Y)
    int pixel_width = m_width_cells * TileHolderWidget::CELL_WIDTH + 
        (m_width_cells - 1) * TileHolderWidget::SPACING;
    int pixel_height = m_height_cells * TileHolderWidget::CELL_HEIGHT + 
        (m_height_cells - 1) * TileHolderWidget::SPACING;
    SetWidth(pixel_width);
    SetHeight(pixel_height);
}

// Updated constructor that extracts dimensions and populates from JSON
TileWidget::TileWidget(JsonObject tileData)
    // Initialize members using data extracted from JSON
    : m_width_cells(tileData["width"] | 1), // Default to 1 if missing
      m_height_cells(tileData["height"] | 1), // Default to 1 if missing
      m_onTouchScript(tileData["onTouch"] | "") // Read onTouch script from JSON
{
    // --- Size Calculation (uses extracted m_width_cells, m_height_cells) ---
    int pixel_width = m_width_cells * TileHolderWidget::CELL_WIDTH +
        (m_width_cells - 1) * TileHolderWidget::SPACING;
    int pixel_height = m_height_cells * TileHolderWidget::CELL_HEIGHT +
        (m_height_cells - 1) * TileHolderWidget::SPACING;
    SetWidth(pixel_width);
    SetHeight(pixel_height);
    Serial.printf("TileWidget created with size (%d, %d) pixels\n", pixel_width, pixel_height);

    // --- Population Logic (remains the same) ---
    if (!tileData["children"].is<JsonArray>()) {
        // No children array or it's not an array, nothing to populate
        return;
    }

    JsonArray childrenArray = tileData["children"].as<JsonArray>();

    for (JsonVariant childVariant : childrenArray) {
        JsonObject childData = childVariant.as<JsonObject>();
        if (childData.isNull()) continue;

        Widget* new_child = WidgetFactory::createWidget(childData); // Use WidgetFactory

        if (new_child) {
            Children.push_back(new_child);
            const char* type = childData["type"] | "unknown"; // Get type for logging
            Serial.printf("Added child type '%s' to TileWidget.\n", type);
        } else {
             const char* type = childData["type"] | "unknown"; // Get type for logging
             Serial.printf("Error: Failed to create child widget object for type '%s'.\n", type);
        }
    }
}

bool TileWidget::HandleTouch(uint16_t x, uint16_t y)
{
    Serial.printf("TileWidget::HandleTouch at (%d, %d)\n", x, y);
    if (m_onTouchScript.isEmpty()) return TouchChildren(x,y);
    
    // Execute script and return its result
    Serial.printf("TileWidget::HandleTouch script: %s\n", m_onTouchScript.c_str());
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

//void TileWidget::Render()
//{
    // 1. Draw Tile Background/Border (Example: simple rectangle)
    // Adjust coordinates to be relative to the parent (TileHolderWidget)
//    Region area = { GetX(), GetY(), GetWidth(), GetHeight() };
//    Display.DrawRect(area, COLOR_BLACK, false); // Draw outline

    // 2. Render Children (calls RenderChildren from ContainerWidget)
//    ContainerWidget::Render(); // Use base class implementation to render children
//}


