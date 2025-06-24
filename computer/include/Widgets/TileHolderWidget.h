#ifndef TILEHOLDERWIDGET_H
#define TILEHOLDERWIDGET_H

#include <ArduinoJson.h>
#include "ContainerWidget.h"
#include "TileWidget.h"

class TileHolderWidget : public ContainerWidget 
{
public:
    static constexpr int GRID_WIDTH = 3;
    static constexpr int GRID_HEIGHT = 10;
    static constexpr int CELL_WIDTH = 180;
    static constexpr int CELL_HEIGHT = 88;
    static constexpr int SPACING = 0;

    TileHolderWidget();
    // Constructor to populate from JSON
    TileHolderWidget(JsonObject config);
    virtual ~TileHolderWidget();

    // Attempts to add a tile at the specified cell position.
    // Returns true on success, false if the space is occupied or out of bounds.
    // Takes ownership of the tile pointer if successful.
    bool AddTile(TileWidget* tile, int cell_x, int cell_y);

protected:
    // Grid to track occupied cells. true means occupied.
    bool m_occupied_cells[GRID_HEIGHT][GRID_WIDTH];
};

#endif // TILEHOLDERWIDGET_H