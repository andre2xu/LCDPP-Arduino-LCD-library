#include <Arduino.h>

// shift register pins
#define data_pin 13
#define latch_pin 7
#define clock_pin 6

// LCD pins
#define lcd_rs_pin 12
#define lcd_rw_pin 11
#define lcd_enable_pin 10
#define lcd_db7 5
#define lcd_db6 4
#define lcd_db5 3
#define lcd_db4 2

void shift_register_function(byte binary_data) {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, binary_data);
    digitalWrite(latch_pin, HIGH);
}

#include <arduino_lcdpp.h> // this header file should only be included AFTER the shift register function declaration



void setup() {
    pinMode(data_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);

    // no shift register (requires data pins)
    LCD_Setup mylcd(lcd_rs_pin, lcd_rw_pin, lcd_enable_pin, 4, true, false, lcd_db7, lcd_db6, lcd_db5, lcd_db4);

    // with shift register (requires a custom shift register function)
    LCD_Setup mylcd(lcd_rs_pin, lcd_rw_pin, lcd_enable_pin, 4, false, false, shift_register_function, true);
}

void loop() {

}