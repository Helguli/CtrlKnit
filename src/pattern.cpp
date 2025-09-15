#include "pattern.h"

#define MAX_COLOR_COUNT 10
#define MAX_PATTERN_WIDTH 200
#define MAX_PATTERN_HEIGHT 300

namespace {
    uint8_t pattern_buffer[MAX_PATTERN_HEIGHT][MAX_PATTERN_WIDTH];
    uint16_t colors[MAX_COLOR_COUNT];
    int color_count = 0;
    int pattern_height = 0;
    int pattern_width = 0;
    struct {
        bool invert = 0;
        bool reverse = 0;
        bool single_motif = 0;
        bool vertical_exp = 0;
        bool horizontal_exp = 0;
        bool mirror = 0;
        bool chevron = 0;
        bool upside_down = 0;
    } settings;
}

namespace pattern {

    const char *option_names[] =
    {
        "Invert              ",
        "Reverse             ",
        "Single motif        ",
        "Vertical expansion  ",
        "Horizontal expansion",
        "Mirror              ",
        "Chevron             ",
        "Upside down         ",
    };

    bool get_option(int option) {
        switch (option) {
            case invert:
                return settings.invert;
                break;
            case reverse:
                return settings.reverse;
                break;
            case single_motif:
                return settings.single_motif;
                break;
            case vertical_exp:
                return settings.vertical_exp;
                break;
            case horizontal_exp:
                return settings.horizontal_exp;
                break;
            case mirror:
                return settings.mirror;
                break;
            case chevron:
                return settings.chevron;
                break;
            case upside_down:
                return settings.upside_down;
                break;
            default:
                return false;
        }
    }
    void switch_option(int option) {
        switch (option) {
            case invert:
                settings.invert = !settings.invert;
                break;
            case reverse:
                settings.reverse = !settings.reverse;
                break;
            case single_motif:
                settings.single_motif = !settings.single_motif;
                break;
            case vertical_exp:
                settings.vertical_exp = !settings.vertical_exp;
                break;
            case horizontal_exp:
                settings.horizontal_exp = !settings.horizontal_exp;
                break;
            case mirror:
                settings.mirror = !settings.mirror;
                break;
            case chevron:
                settings.chevron = !settings.chevron;
                break;
            case upside_down:
                settings.upside_down = !settings.upside_down;
                break;
        }
    }
    int width() {
        return pattern_width;
    }
    int height() {
        return pattern_height;
    }
    int new_row(uint16_t *buffer, int size) {
        if (pattern_height > MAX_PATTERN_HEIGHT) {
            return 1;
        }
        if (size > NEEDLE_COUNT) {
            return 1;
        }
        pattern_width = size;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < color_count; j++) {
                if (buffer[i] == colors[j]) {
                    pattern_buffer[pattern_height][i] = j;
                    goto loop_end;
                }
            }
            if (color_count == MAX_COLOR_COUNT) {
                return 1;
            }
            pattern_buffer[pattern_height][i] = color_count;
            colors[color_count] = buffer[i];
            color_count++;
            loop_end:;
        }
        pattern_height++;
        return 0;
    }

    void new_file() {
        color_count = 0;
        pattern_height = 0;
        pattern_width = 0;
    }

    uint8_t get_color(int row, int column) {
        if (pattern_height == 0) {
            return settings.invert ? 1 : 0;
        }
        if (settings.vertical_exp) {
            row = row / 2;
        }
        if (settings.horizontal_exp) {
            column = column / 2;
        }
        if (settings.single_motif && (row >= pattern_height || column >= pattern_width || column < 0)) {
            return settings.invert ? 1 : 0;
        }
        if (settings.chevron) {
            while (column < 0) {
                column += pattern_width * 2 - 2;
            }
            column = column % (pattern_width * 2 - 2);
            if (column >= pattern_width) {
                column = pattern_width * 2 - column - 2;
            }
        }
        if (settings.mirror) {
            while (column < 0) {
                column += pattern_width * 2;
            }
            column = column % (pattern_width * 2);
            if (column >= pattern_width) {
                column = pattern_width * 2 - column - 1;
            }
        }
        row = row % pattern_height;
        while (column < 0) {
            column += pattern_width;
        }
        column = column % pattern_width;
        if (settings.reverse) {
            column = pattern_width - column - 1;
        }
        if (!settings.upside_down) {
            row = pattern_height - row - 1;
        }
        return settings.invert ? !pattern_buffer[row][column] : pattern_buffer[row][column];

    }

}
