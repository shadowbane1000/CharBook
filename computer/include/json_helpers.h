#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H

#include <ArduinoJson.h> // For JsonVariant
#include <stdint.h>      // For int16_t

namespace JsonHelpers {

// Parses a JSON value that can be an integer (pixels) or a string
// with units 'c' (cells) or 'px'/'p' (pixels).
// Returns the dimension in pixels.
int16_t parseDimension(JsonVariant value, int cell_dimension, int spacing);

} // namespace JsonHelpers

#endif // JSON_HELPERS_H
