#include <Arduino.h>

#include <PNGdec.h>


#include "ui.h"
#include "pattern.h"

// UI pins
#define UP_ENC 1
#define DOWN_ENC 2
#define SELECT_BUTTON 21
#define KNIT_BUTTON 38
#define MENU_BUTTON 37
#define BACK_BUTTON 36

// Knitting machine pins
#define ND1 4 // Needle 1 marker
#define KSL 5 // Pattern start/stop markers
#define DOB 6 // Data out buffer
#define CCP 8 // Clock pulse signal (every new needle)
#define HOK 7 // Carriage direction (right = 0)

//Others
#define LEFT 1
#define RIGHT 0

#define INTERRUPT_VALUE 50

#define LED_PIN 48


hw_timer_t * timer = NULL;

volatile int direction;
volatile int current_needle;
volatile int row_number = 0;
volatile int pattern_start;
volatile int pattern_end;
volatile bool inside_pattern = false;
volatile bool knitting = false;
volatile int blink = 0;

volatile bool prev_nd1_state = 1;
volatile bool prev_ksl_state = 1;
volatile bool prev_ccp_state = 1;
volatile bool prev_hok_state = 1;

volatile bool prev_up_state = 1;
volatile bool prev_down_state = 1;
volatile bool prev_select_state = 1;
volatile bool prev_knit_state = 1;
volatile bool prev_menu_state = 1;
volatile bool prev_back_state = 1;

volatile bool up_pushed = 0;
volatile bool down_pushed = 0;
volatile bool select_pushed = 0;
volatile bool knit_pushed = 0;
volatile bool menu_pushed = 0;
volatile bool back_pushed = 0;
volatile bool row_changed = 0;

//reading inputs, patterning
void IRAM_ATTR onTimer() {

    direction = (GPIO.in >> HOK) & 0x1;

    int ccp_state = (GPIO.in >> CCP) & 0x1;


    if (ccp_state != prev_ccp_state) {
        prev_ccp_state = ccp_state;
        if (ccp_state) {
            if (direction == RIGHT) {
                current_needle++;
            } else {
                current_needle--;
            }
            if (knitting && inside_pattern) {
                if (pattern::get_color(row_number, current_needle)) {
                    GPIO.out_w1ts = (1 << DOB);
                } else {
                    GPIO.out_w1tc = (1 << DOB);
                }
            }
        }
    }

    int ksl_state = (GPIO.in >> KSL) & 0x1;
    if (inside_pattern != ksl_state) {
        inside_pattern = ksl_state;
        if (inside_pattern) {
            if (direction == RIGHT) {
                current_needle = pattern_start - 1;
            } else {
                current_needle = pattern_end + 1;
            }
        } else {
            if (direction == RIGHT) {
                pattern_end = current_needle;
            } else {
                pattern_start = current_needle;
            }
            row_number++;
            row_changed = 1;
            //digitalWrite(DOB, LOW);
            GPIO.out_w1tc = (1 << DOB);
        }
    }


    int nd1_state = (GPIO.in >> ND1) & 0x1;
    if (prev_nd1_state != nd1_state)
    {
        prev_nd1_state = nd1_state;
        if (nd1_state == LOW)
        {
            if (direction == RIGHT)
            {
                current_needle = -1;
            }
            else
            {
                // Cannot set the first needle when the direction is left.
                //current_needle = -1;
                Serial.println(current_needle);
            }
        }
    }



    int up_state = (GPIO.in >> UP_ENC) & 0x1;
    int down_state = (GPIO.in >> DOWN_ENC) & 0x1;
    int select_state = (GPIO.in >> SELECT_BUTTON) & 0x1;
    int knit_state = (GPIO.in1.val >> (KNIT_BUTTON - 32)) & 0x1;
    int menu_state = (GPIO.in1.val >> (MENU_BUTTON - 32)) & 0x1;
    int back_state = (GPIO.in1.val >> (BACK_BUTTON - 32)) & 0x1;
    if (prev_up_state != up_state && !up_state && down_state) {
        up_pushed = 1;
    }
    if (prev_down_state != down_state && !down_state && up_state) {
        down_pushed = 1;
    }
    if (prev_select_state != select_state && select_state == 0) {
        select_pushed = 1;
    }
    if (prev_knit_state != knit_state && knit_state == 0) {
        knit_pushed = 1;
    }
    if (prev_menu_state != menu_state && menu_state == 0) {
       menu_pushed = 1;
    }
    if (prev_back_state != back_state && back_state == 0) {
        back_pushed = 1;
    }

    prev_up_state = up_state;
    prev_down_state = down_state;
    prev_select_state = select_state;
    prev_knit_state = knit_state;
    prev_menu_state = menu_state;
    prev_back_state = back_state;
    if (knitting) {
        blink++;
        if (blink > 1000000) {
            blink = 0;
        }
    }
    if (knitting && inside_pattern || knitting && blink < 500000 / INTERRUPT_VALUE) {
        GPIO.out1_w1ts.val = (1U << (LED_PIN - 32));
    } else {
        GPIO.out1_w1tc.val = (1U << (LED_PIN - 32));
    }
}



// Setup - setting modes for I/O pins, setting timer and calling the UI setup function
void setup() {
    pinMode(UP_ENC, INPUT);
    pinMode(DOWN_ENC, INPUT);
    pinMode(SELECT_BUTTON, INPUT);
    pinMode(KNIT_BUTTON, INPUT);
    pinMode(MENU_BUTTON, INPUT);
    pinMode(BACK_BUTTON, INPUT);
    pinMode(ND1, INPUT);
    pinMode(KSL, INPUT);
    pinMode(CCP, INPUT);
    pinMode(HOK, INPUT);
    pinMode(DOB, OUTPUT);
    digitalWrite(DOB, LOW);
    pinMode(LED_PIN, OUTPUT);
    timer = timerBegin(0, 8000, true);  // 80 MHz / 8000 = 10 kHz
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, INTERRUPT_VALUE, true); // 50 µs * 100 = 0.005s
    timerAlarmEnable(timer);

    Serial.begin(115200);
    while (!Serial) {
        delay(10);  // Wait for USB serial to connect
    }
    Serial.println("USB CDC Serial working!");
    ui::ui_setup();

}


void loop() {
    if (up_pushed) {
        up_pushed = 0;
        ui::up_pressed();
    }
    if (down_pushed) {
        down_pushed = 0;
        ui::down_pressed();
    }
    if (select_pushed) {
        select_pushed = 0;
        ui::select_pressed();
    }
    if (knit_pushed) {
        knit_pushed = 0;
        knitting = !knitting;
        row_number = 0;
    }
    if (menu_pushed) {
        menu_pushed = 0;
        Serial.println("pushed5");
        ui::menu_pressed();
    }

    if (row_changed) {
        row_changed = 0;
        bool empty = true;
        int first_color = pattern::get_color(row_number, 0);
        for (int i = 0; i < pattern::width(); i++) {
            Serial.print(pattern::get_color(row_number, i));
            if (pattern::get_color(row_number, i) != first_color) {
                empty = false;
            }
        }
        if ((row_number % pattern::height()) - 2 == pattern::height()) {
            ui::warning_signal();
        } else if (empty) {
            ui::empty_row();
        }
        Serial.print("\n");
        Serial.println("Pattern stopped.");
        Serial.println(current_needle);
    }


}
