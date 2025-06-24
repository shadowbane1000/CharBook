#ifndef TILEWIDGET_H
#define TILEWIDGET_H

#include <ArduinoJson.h> // Include for JsonObject
#include "ContainerWidget.h" // Assuming a base Widget class exists

class TileWidget : public ContainerWidget {
public:
    // Constructor takes size in cells
    TileWidget(int width_cells, int height_cells);
    // Constructor takes JSON data for configuration and population
    TileWidget(JsonObject tileData); // Changed signature
    virtual ~TileWidget() = default;

    virtual bool HandleTouch(uint16_t x, uint16_t y) override;

    int GetWidthCells() const { return m_width_cells; }
    int GetHeightCells() const { return m_height_cells; }

    // Set by TileHolderWidget
protected:
    int m_width_cells;
    int m_height_cells;
    String m_onTouchScript; // Script to execute on touch
};

#endif // TILEWIDGET_H