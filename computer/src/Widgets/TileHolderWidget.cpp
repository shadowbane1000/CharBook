#include <Arduino.h> // For Serial output
#include <ArduinoJson.h>
#include "Widgets/TileHolderWidget.h"
#include "Widgets/TileWidget.h" // Make sure TileWidget header is included

TileHolderWidget::TileHolderWidget() : ContainerWidget() {
    // Initialize the grid - all cells are initially empty
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            m_occupied_cells[y][x] = false;
        }
    }
    // Set the holder's own size based on the grid (optional, depends on base Widget)
    SetWidth(GRID_WIDTH * CELL_WIDTH + (GRID_WIDTH + 1) * SPACING);
    SetHeight(GRID_HEIGHT * CELL_HEIGHT + (GRID_HEIGHT + 1) * SPACING);
}

TileHolderWidget::TileHolderWidget(JsonObject config) : ContainerWidget(config) {
    // Initialize the grid - all cells are initially empty
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            m_occupied_cells[y][x] = false;
        }
    }
    // Set the holder's own size based on the grid
    SetWidth(GRID_WIDTH * CELL_WIDTH + (GRID_WIDTH + 1) * SPACING);
    SetHeight(GRID_HEIGHT * CELL_HEIGHT + (GRID_HEIGHT + 1) * SPACING);
    Serial.printf("TileHolderWidget size %d %d\n", GetWidth(), GetHeight());

    auto tiles = config["tiles"].as<JsonArray>();

    for(JsonVariant tileVariant : tiles)
    {
        JsonObject tileData = tileVariant.as<JsonObject>();
        if (tileData.isNull()) {
            Serial.println("Warning: Skipping null or invalid tile entry in JSON array.");
            continue;
        }

        int cell_x = tileData["x"] | -1;
        int cell_y = tileData["y"] | -1;

        if (cell_x < 0 || cell_y < 0) {
            Serial.printf("Error: Invalid or missing x/y for tile. Skipping.\n");
            continue;
        }

        // --- Create TileWidget using the new constructor ---
        // Serial.printf("Processing tile: pos=(%d,%d), size=(%dx%d)\n", ...); // Size is now internal to TileWidget constructor

        TileWidget* new_tile = nullptr;
        // Use the constructor that takes only JsonObject
        new_tile = new TileWidget(tileData);

        // --- Add the created tile to the grid ---
        if (!AddTile(new_tile, cell_x, cell_y)) {
            Serial.printf("Error: Failed to add tile at (%d,%d). Deleting tile.\n", cell_x, cell_y);
            delete new_tile; // Clean up if AddTile failed
        }
    }
}

TileHolderWidget::~TileHolderWidget() {
}

bool TileHolderWidget::AddTile(TileWidget* tile, int cell_x, int cell_y) {
    if (!tile) {
        Serial.println("Error: Attempted to add null tile.");
        return false;
    }

    // Get width/height *after* the tile has been constructed
    int tile_width_cells = tile->GetWidthCells();
    int tile_height_cells = tile->GetHeightCells();

    // 1. Check Bounds
    if (cell_x < 0 || cell_y < 0 ||
        cell_x + tile_width_cells > GRID_WIDTH ||
        cell_y + tile_height_cells > GRID_HEIGHT)
    {
        Serial.printf("Error: Tile placement (%d,%d) with size (%dx%d) is out of grid bounds (%dx%d).\n",
                      cell_x, cell_y, tile_width_cells, tile_height_cells, GRID_WIDTH, GRID_HEIGHT);
        return false;
    }

    // 2. Check Collision
    for (int y = cell_y; y < cell_y + tile_height_cells; ++y) {
        for (int x = cell_x; x < cell_x + tile_width_cells; ++x) {
            if (m_occupied_cells[y][x]) {
                Serial.printf("Error: Collision detected at cell (%d,%d) while placing tile at (%d,%d).\n",
                              x, y, cell_x, cell_y);
                return false; // Found an occupied cell where the new tile would go
            }
        }
    }

    // 3. Place Tile and Mark Cells
    for (int y = cell_y; y < cell_y + tile_height_cells; ++y) {
        for (int x = cell_x; x < cell_x + tile_width_cells; ++x) {
            m_occupied_cells[y][x] = true;
        }
    }

    // 4. Calculate and Set Pixel Position
    int pixel_x = cell_x * (CELL_WIDTH + SPACING);
    int pixel_y = cell_y * (CELL_HEIGHT + SPACING);
    tile->SetX(pixel_x);
    tile->SetY(pixel_y);

    // 6. Add to Children (assuming ownership)
    Children.push_back(tile);

    Serial.printf("Tile added successfully at cell (%d,%d) with pixel pos (%d,%d).\n",
                  cell_x, cell_y, tile->GetX(), tile->GetY());

    return true;
}