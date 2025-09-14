#pragma once

#include <Arduino.h>

#define NEEDLE_COUNT 200

namespace pattern {
    enum pattern_options : int {
        invert,
        reverse,
        single_motif,
        vertical_exp,
        horizontal_exp,
        mirror,
        chevron,
        upside_down,
    };
    extern const char *option_names[];
    bool get_option(int option);
    void switch_option(int option);
    int width();
    int height();
    int new_row(uint16_t *buffer, int size);
    uint8_t IRAM_ATTR get_color(int row, int column);
}
