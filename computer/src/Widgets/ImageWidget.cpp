#include "Widgets/ImageWidget.h"
#include <Arduino.h> // For Serial
#include "display.h" // For Display class
#include "image.h" // Include Image header for image.width/height
#include "Widgets/TileHolderWidget.h" // Include for CELL_WIDTH, CELL_HEIGHT, SPACING
#include "json_helpers.h" // Include the new helper header
#include "lua_env.h"

ImageWidget::ImageWidget(JsonObject config)
    : Widget(config),
      m_image_path(config["path"] | ""),
      m_onTouchScript(config["onTouch"] | "") // Read onTouch script from JSON
{
    // Extract properties from JSON
    m_image_path = config["path"] | ""; // Get image path, default to empty string

    // Parse position and dimensions using the helper function
    SetX(JsonHelpers::parseDimension(config["x"], TileHolderWidget::CELL_WIDTH, TileHolderWidget::SPACING));
    SetY(JsonHelpers::parseDimension(config["y"], TileHolderWidget::CELL_HEIGHT, TileHolderWidget::SPACING));
    int16_t config_w = JsonHelpers::parseDimension(config["width"], TileHolderWidget::CELL_WIDTH, TileHolderWidget::SPACING);
    int16_t config_h = JsonHelpers::parseDimension(config["height"], TileHolderWidget::CELL_HEIGHT, TileHolderWidget::SPACING);

    if (m_image_path.isEmpty()) {
        Serial.println("Error: ImageWidget created with no 'path' in JSON config.");
        // Set dimensions from config even if path is missing
        SetWidth(config_w);
        SetHeight(config_h);
        return; // Don't proceed if path is missing
    }

    if(!m_image_path.endsWith(".png")) m_image_path += ".png";
    image.Load(m_image_path.c_str());

    // Set width and height based on image if not specified in config
    SetWidth((config_w == 0 && image.Initialized) ? image.width : config_w);
    SetHeight((config_h == 0 && image.Initialized) ? image.height : config_h);
}

ImageWidget::ImageWidget(const char* path, int16_t x_pos, int16_t y_pos, int16_t width, int16_t height)
    : Widget(),
      m_image_path(path)
{
    SetX(x_pos);
    SetY(y_pos);
    // Temporarily store provided width/height
    int16_t provided_w = width;
    int16_t provided_h = height;

    if (m_image_path.isEmpty()) {
        Serial.println("Error: ImageWidget created with an empty path.");
        // Set w/h from provided values even if path is empty
        SetWidth(provided_w);
        SetHeight(provided_h);
        return;
    }

    if(!m_image_path.endsWith(".png")) m_image_path += ".png";
    image.Load(m_image_path.c_str());

    // Set width and height based on image if not specified (i.e., 0)
    SetWidth((provided_w == 0 && image.Initialized) ? image.width : provided_w);
    SetHeight((provided_h == 0 && image.Initialized) ? image.height : provided_h);
}

ImageWidget::ImageWidget(uint8_t* data, uint16_t size, int16_t x_pos, int16_t y_pos, int16_t width, int16_t height)
    : Widget()
{
    SetX(x_pos);
    SetY(y_pos);
    // Temporarily store provided width/height
    int16_t provided_w = width;
    int16_t provided_h = height;

    image.Static(data, size);

    SetWidth((provided_w == 0) ? image.width : provided_w);
    SetHeight((provided_h == 0) ? image.height : provided_h);
}

void ImageWidget::Render(int16_t parentx, int16_t parenty)
{
    Serial.printf("ImageWidget::Render: Image='%s', x=%d, y=%d, w=%d, h=%d i=%s\n",
                   m_image_path.c_str(), GetX(), GetY(), GetWidth(), GetHeight(),
                   image.Initialized ? "true" : "false");
    int absoluteX = parentx + GetX();
    int absoluteY = parenty + GetY();

    if(image.Initialized)
    {
        Display.DrawImage(&image, absoluteX, absoluteY);
    }
}

bool ImageWidget::HandleTouch(uint16_t x, uint16_t y)
{
    Serial.printf("ImageWidget::HandleTouch at (%d, %d)\n", x, y);
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
