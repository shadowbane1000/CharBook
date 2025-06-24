#include "json_helpers.h"
#include <Arduino.h> // For String, .toInt(), .endsWith(), .substring(), .trim()

namespace JsonHelpers {

int16_t parseDimension(JsonVariant value, int cell_dimension, int spacing) {
    if (value.is<int16_t>() || value.is<int>()) {
        int16_t num_part = value.as<int16_t>();
        if(num_part == 0) return 0;
        return num_part * cell_dimension + (num_part - 1) * spacing;
    } else if (value.is<const char*>() || value.is<String>()) {
        String str_val = value.as<String>();
        str_val.trim(); // Remove leading/trailing whitespace
        int num_part = 0;
        int16_t pixel_val = 0;

        if (str_val.endsWith("c")) {
            num_part = str_val.substring(0, str_val.length() - 1).toInt();
            if (num_part > 0) {
                // Calculate pixel size based on cells and spacing
                pixel_val = num_part * cell_dimension + (num_part - 1) * spacing;
            } else {
                 pixel_val = 0; // Or handle 0 cells case if needed
            }
        } else if (str_val.endsWith("px")) {
            num_part = str_val.substring(0, str_val.length() - 2).toInt();
            pixel_val = num_part;
        } else if (str_val.endsWith("p")) {
             num_part = str_val.substring(0, str_val.length() - 1).toInt();
             pixel_val = num_part;
        } else {
            // Assume cells if no suffix or invalid suffix
            num_part = str_val.toInt();
            if (num_part > 0) {
                // Calculate pixel size based on cells and spacing
                pixel_val = num_part * cell_dimension + (num_part - 1) * spacing;
            } else {
                 pixel_val = 0; // Or handle 0 cells case if needed
            }
        }
        return pixel_val;
    }
    return 0; // Default if type is unexpected or value is missing/null
}

} // namespace JsonHelpers
