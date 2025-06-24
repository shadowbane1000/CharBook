#ifndef LUA_VARIANT_H
#define LUA_VARIANT_H

#include <string.h> // For strdup
#include <stdlib.h> // For free

// It's recommended to place LuaReturnType and LuaValueVariant definitions in lua_env.h
// or a separate header file (e.g., lua_variant.h) so that calling code can use them.
enum class LuaReturnType {
    NIL,
    BOOLEAN,
    NUMBER,
    STRING,
    TABLE,
    FUNCTION,
    USERDATA,
    THREAD,
    LIGHTUSERDATA,
    ERROR // For script execution errors
};

struct LuaValueVariant {
    LuaReturnType type;
    union {
        bool b_val;
        double d_val;
        char* s_val; // Dynamically allocated for STRING type
    } value;
    char* error_message; // Dynamically allocated for ERROR type

    LuaValueVariant() : type(LuaReturnType::NIL), error_message(nullptr) {
        value.s_val = nullptr; // Initialize union member to a known state
    }

    ~LuaValueVariant() {
        clear_resources();
    }

    // Move constructor
    LuaValueVariant(LuaValueVariant&& other) noexcept
        : type(other.type), value(other.value), error_message(other.error_message) {
        // Pilfer resources from other
        other.type = LuaReturnType::NIL; // Mark other as moved-from
        other.value.s_val = nullptr;
        other.error_message = nullptr;
    }

    // Move assignment operator
    LuaValueVariant& operator=(LuaValueVariant&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        clear_resources(); // Clear current resources

        // Pilfer resources from other
        type = other.type;
        value = other.value;
        error_message = other.error_message;

        other.type = LuaReturnType::NIL; // Mark other as moved-from
        other.value.s_val = nullptr;
        other.error_message = nullptr;
        return *this;
    }

    // Delete copy constructor and copy assignment operator to prevent shallow copies
    // If copying is needed, a proper deep copy implementation is required.
    LuaValueVariant(const LuaValueVariant&) = delete;
    LuaValueVariant& operator=(const LuaValueVariant&) = delete;

private:
    void clear_resources() {
        if (type == LuaReturnType::STRING && value.s_val != nullptr) {
            free(value.s_val);
            value.s_val = nullptr;
        }
        if (error_message != nullptr) {
            free(error_message);
            error_message = nullptr;
        }
    }
};

#endif // LUA_VARIANT_H
