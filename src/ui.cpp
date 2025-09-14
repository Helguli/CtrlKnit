#include "ui.h"

#include <SPI.h>

#define SD_FAT_TYPE 3
#define SPI_SPEED SD_SCK_MHZ(4)
#include <SdFat.h>

#include "pattern.h"

#define TEXT_SIZE 2
#define TEXT_HEIGHT 8


// SPI pins

// MOSI = SPID
// MISO = SPIQ

#define BUZZER_PIN 35   // GPIO pin where the buzzer is connected
#define LEDC_CHANNEL 0  // LEDC channel (0–15)


// TFT pins
#define TFT_CS        10
#define TFT_RST       9
#define TFT_MOSI      11 // SDA // FSPID // fixed
#define TFT_DC        46 // A0 // MISO
#define TFT_SCK       12 // FSPICLK // fixed

// SD pins
#define SD_MOSI 16
#define SD_MISO 17
#define SD_SCK 14
#define SD_SS 15

SPIClass sd_spi(HSPI);
SPIClass tft_spi(FSPI);

// SPI config for SD card.
SdSpiConfig cfg(SD_SS, 1, SPI_SPEED, &sd_spi);


namespace {


    SdFs sd;
    FsFile file;

    String menu_items[] = {
        "Invert",
        "Reverse",
        "Single motif",
        "Vertical expansion",
        "Horizontal expansion",
        "Mirror",
        "Chevron",
    };

    int selected_menu = 0;



    //File myfile;
    String entries[100]; // Buffer for directory entries
    int entryCount = 0;

    String currentDir;
    int selectedIndex = 0;
    uint16_t color1 = 0;
    uint16_t color2 = 0;
    int png_width = 0;
    int png_height = 0;

    int ui_width = 0;
    int ui_height = 0;
    int text_rows = 0;

    bool menu = 0;


    //Adafruit_ST7735 tft = Adafruit_ST7735(&tft_spi, TFT_CS, TFT_DC, TFT_RST);
    Adafruit_ST7796S tft = Adafruit_ST7796S(&tft_spi, TFT_CS, TFT_DC, TFT_RST);
    //Adafruit_ST7796S tft;

    PNG png;

    void * myOpen(const char *filename, int32_t *size) {
        Serial.printf("Attempting to open %s\n", filename);
        file.open(filename, O_RDONLY);
        *size = file.fileSize();
        return &file;
    }
    void myClose(void *handle) {
        if (file) file.close();
    }
    int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
        if (!file) return 0;
        return file.read(buffer, length);
    }
    int32_t mySeek(PNGFILE *handle, int32_t position) {
        if (!file) return 0;
        return file.seekSet(position);
    }

    // Function to draw pixels to the display
    void PNGDraw(PNGDRAW *pDraw) {
        uint16_t usPixels[320];
        png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
        tft.drawRGBBitmap(0, pDraw->y + TEXT_HEIGHT * TEXT_SIZE * 2, usPixels, pDraw->iWidth, 1);
        Serial.println(pattern::new_row(usPixels, pDraw->iWidth));
        Serial.printf("%d;",pDraw->y);
    }

    void display_menu() {
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.println("Menu");
        for (int i = 0; i < 7; i++) {
            tft.printf("%s%s  %s\n", (selected_menu == i ? "> ": "  "), pattern::option_names[i], (pattern::get_option(i) ? "ON" : "OFF"));
        }
    }

}

namespace ui {
    void clear_screen();
    void dummy();

    void ui_setup() {
        Serial.printf("\nSPI pins:\n");
        Serial.printf("MISO: %d\n", int(MISO));
        Serial.printf("MOSI: %d\n", int(MOSI));
        Serial.printf("SCK:  %d\n", int(SCK));
        Serial.printf("SS:   %d\n", int(SS));
        //tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
        tft.init(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, 0, 0, ST7796S_BGR);         // Init ST7735S chip, black tab
        
        ledcSetup(LEDC_CHANNEL, 2000 /*Hz*/, 8);
        ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
        //ledcWriteNote(LEDC_CHANNEL, melody[i], 4);
        if (ledcWriteNote(LEDC_CHANNEL, NOTE_A, 5)) {
            delay(50);
        }

        ledcWriteTone(LEDC_CHANNEL, 0);

        tft.invertDisplay(true);
        tft.setRotation(3);
        tft.fillScreen(ST77XX_RED);
        tft.setTextColor(ST77XX_BLACK);
        tft.setTextSize(2);
        Serial.println("KnitKnotroll");
        Serial.println(tft.width());
        Serial.println(tft.height());
        

        ui_width = tft.width();
        ui_height = tft.height();
        text_rows = ui_height / TEXT_HEIGHT / TEXT_SIZE;


        tft.fillScreen(ST77XX_WHITE);
        tft.setCursor(0, 0);


        sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);
        if (!sd.begin(cfg)) {
            if (sd.card()->errorCode()) {
                tft.println("SD initialization failed.");
                tft.println("Is the card correctly inserted?");
                Serial.println("SD initialization failed.");
                Serial.println("Is the card correctly inserted?");
                return;
            }
            tft.println("Card successfully initialized. :)");
            if (sd.vol()->fatType() == 0) {
                tft.println("Can't find a valid FAT16/FAT32/exFAT partition.");
                tft.println("Please reformat the card.");
                Serial.println("Can't find a valid FAT16/FAT32/exFAT partition.");
                Serial.println("Please reformat the card.");
                return;
            }
            Serial.println("test3");
            tft.println("Can't determine error type.");
            return;
        }
        Serial.println("SD card mounted. :)");
        tft.setTextColor(ST77XX_RED);
        tft.println("SD card mounted. :)");

        
  

        //while (!SD.begin(10)) {
        //    Serial.println("Unable to access SD Card");
        //    Serial.println(MOSI);//11
        //    Serial.println(MISO);//13
        //    Serial.println(SCK);//12
        //    Serial.println(SS);//10
        //    tft.println("Unable to access SD Card");
        //    delay(1000);
        //}
        readDirectory("/");
        displayDirectory();
    }

// Functions to access a file on the SD card

    void readDirectory(const char *dirname) {
        Serial.println("1");
        FsFile dir;
        if (!dir.open(dirname)) {
            tft.printf("Could not open directory:\n%s\n", dirname);
        }
        Serial.println("2");
        entryCount = 0;
        char arr[200];
        Serial.println("3");
        FsFile f;
        Serial.println("3a");
        while (f.openNext(&dir, O_RDONLY)) {
            Serial.println("3b");
            f.getName(arr, 200);
            entries[entryCount++] = String(arr);
            f.close();
        }
        Serial.println("4");
        dir.close();
        Serial.println("5");
    }

    /*
     * TODO Write efficient directory list.
     * It takes a lot of thime to write to the screen.
     */
    void displayDirectory() {
        tft.fillScreen(ST77XX_WHITE);
        tft.setCursor(0, 0);
        tft.println("Files:");
        int start = 0, end = entryCount;
        if (selectedIndex > text_rows - text_rows / 2) {
            start = selectedIndex - text_rows / 2;
        }
        end = min(entryCount, start + text_rows);

        for (int i = start; i < end; i++) {
            if (i == selectedIndex) {
                //Serial.print("> ");
                tft.print("> ");
            } else {
                tft.print("  ");
            }
            //Serial.println(entries[i]);
            tft.println(entries[i]);
        }
    }

    void openPNG() {
        const char *name = entries[selectedIndex].c_str();
        const int len = strlen(name);
        if (len > 3 && (strcmp(name + len - 3, "PNG") == 0 || strcmp(name + len - 3, "png") == 0)) {
            Serial.print("File: ");
            Serial.println(name);
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(0, 0);
            Serial.print("File: ");
            Serial.println(name);
            tft.print("File: ");
            tft.println(name);
            String strname = name;
            strname = "/" + strname;
            int rc = png.open(strname.c_str(), myOpen, myClose, myRead, mySeek, PNGDraw);
            if (rc == PNG_SUCCESS) {
                Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
                png_width = png.getWidth();
                png_height = png.getHeight();
                tft.printf("w: %d, h: %d", png_width, png_height);
                rc = png.decode(NULL, 0);
                png.close();
            } else {
                tft.println("Could not open file!");
            }
        }
    }

    void up_pressed() {
        if (menu) {
            if (selected_menu > 0) {
                selected_menu--;
                display_menu();
            }

        } else {
            if (selectedIndex > 0) {
                selectedIndex--;
                displayDirectory();
            }
        }
    }
    void down_pressed() {
        if (menu) {
            if (selected_menu < 6) {
                selected_menu++;
                display_menu();
            }

        } else {
            if (selectedIndex < entryCount - 1) {
                selectedIndex++;
                displayDirectory();
            }
        }
    }
    void select_pressed() {
        if (menu) {
            pattern::switch_option(selected_menu);
            display_menu();

        } else {
            ui::openPNG();
        }
    }
    void back_pressed();
    void menu_pressed() {
        menu = !menu;
        if (menu) {
            display_menu();
        } else {
            displayDirectory();
        }
    }

    void warning_signal()
    {
        if (ledcWriteNote(LEDC_CHANNEL, NOTE_A, 4)) {
            delay(50);
        }
        ledcWriteTone(LEDC_CHANNEL, 0);
    }
    void empty_row() {
        if (ledcWriteNote(LEDC_CHANNEL, NOTE_A, 4)) {
            delay(100);
        }
        if (ledcWriteNote(LEDC_CHANNEL, NOTE_C, 5)) {
            delay(100);
        }
        ledcWriteTone(LEDC_CHANNEL, 0);
    }
}
