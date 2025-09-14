#pragma once

#include <Arduino.h>

#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7796S.h>
#include <SPI.h>


#include <PNGdec.h>

namespace ui {
    void clear_screen();
    void dummy();
    void ui_setup();

    void readDirectory(const char *dirname);

    void displayDirectory();

    void openPNG();

    void up_pressed();
    void down_pressed();
    void select_pressed();
    void back_pressed();
    void menu_pressed();
    void warning_signal();
    void empty_row();
}
