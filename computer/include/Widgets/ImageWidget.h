#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <ArduinoJson.h>
#include "Widget.h"
#include "image.h"

class ImageWidget : public Widget {
public:
    ImageWidget(JsonObject config);
    ImageWidget(const char* path, int16_t x, int16_t y, int16_t w = 0, int16_t h = 0);
    ImageWidget(uint8_t* imageData, uint16_t imageSize, int16_t x, int16_t y, int16_t w = 0, int16_t h = 0);
    virtual ~ImageWidget() = default;

    virtual void Render(int16_t parentx, int16_t parenty) override;
    virtual void Update() override {}

    bool HandleTouch(uint16_t x, uint16_t y) override;
private:
    String m_image_path;
    Image image;
    String m_onTouchScript; // Script to execute on touch
};

#endif // IMAGEWIDGET_H