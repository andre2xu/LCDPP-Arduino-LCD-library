#ifndef ARDUINO_LCDPP_H
#define ARDUINO_LCDPP_H
#pragma once

/*

DL = Data length received by LCD   [0: 4-bit | 1: 8-bit]
N = No. of display lines   [0: 1 display line | 1: 2 display lines *F bit will be ignored]
F = Character font   [0: 5x8 | 1: 5x10]
S/C = Selects cursor OR cursor & display  [0: selects cursor only | 1: selects display & cursor]
R/L = Movement direction of cursor & display  [0: left | 1: right]
D = Turns display on/off  [0: off | 1: on]
C = Turns cursor on/off  [0: off | 1: on]
B = Blinks the ENTIRE character the cursor is on  [0: no blink | 1: blink]
I/D = Increments or decrements address of next character (i.e. cursor position)  [0: decr | 1: incr]
S = Shifts display according to I/D  [0: no shifting | 1: shifting]

References are on pg 26-29 of Hitachi HD44780 datasheet



SETUP NOTES:
- control pins include: register select (rs), read/write (rw), enable (en)
- data pins used for 4-bit mode: db7, db6, db5, db4
- data pins used for 8-bit mode: db7 -> db0
- most significant bit (MSB) = db7 / least significant bit (LSB) = db0

- three possible ways to set up the LCD:
    1. No shift register (i.e. all 3 control pins are directly connected to the Arduino)
    2. Shift register for data pins of LCD but NOT its control pins
    3. Shift register for both control & data pins (N/A)

'rs', 'rw', and 'en' will not be required for the 3rd way


DDRAM & CGRAM NOTES:
- addresses for both are automatically incremented by 1 after writing a character to the display
- to move display lines, first set rs to LOW and rw to LOW (this will switch to LCD command mode), then use the 'set DDRAM & CGRAM address' instructions, from the datasheet, to send 40 hex to both of them. REMEMBER: DDRAM requires 7 bits for its address and CGRAM requires only 6; ***when reading the binary representation of the hex number, start from the right (MSBFIRST)


CURSOR & DISPLAY NOTES:
- cursor position determines where characters are written
- shifting moves all display lines simultaneously
- total of 80 characters allowed:
    1 display line = 80 on the single line & final character is placed to the left of the first character
    2 display lines = 40 per line & final character is placed to the left of the first character of the 2nd line (i.e. bottom left)
- characters are automatically written to the next line once the limit of that line has been reached (41... NOT 40)
- all lines have a difference of 40 hex. Characters can be moved to a new line or a previous one if 40 hex or its multiples are added or subtracted from the current CGRAM & DDRAM addresses

*/

// LCD function set commands
#define funcset_DL8_MSB B00001100
#define funcset_DL8_LSB B00110000
#define funcset_DL4_MSB B00000100
#define funcset_DL4_LSB B00100000

// LCD clear display command
#define clearLCD_DL8_MSB B10000000
#define clearLCD_DL8_LSB B00000001
#define clearLCD_DL4_MSB B00001000
#define clearLCD_DL4_LSB B00010000

// LCD return home command
#define returnLCDHome_DL8_MSB B01000000
#define returnLCDHome_DL8_LSB B00000010
#define returnLCDHome_DL4_MSB B00000100
#define returnLCDHome_DL4_LSB B00100000

// LCD display command (default has display on and no cursor or blink)
#define display1_cursor0_blink0_DL8_MSB B00110000
#define display1_cursor0_blink0_DL8_LSB B00001100
#define display1_cursor0_blink0_DL4_MSB B00000011
#define display1_cursor0_blink0_DL4_LSB B11000000

// LCD entry mode command (default has I/D set to increment & shifting disabled)
#define entryset_incr_shift0_DL8_MSB B01100000
#define entryset_incr_shift0_DL8_LSB B00000110
#define entryset_incr_shift0_DL4_MSB B00000110
#define entryset_incr_shift0_DL4_LSB B01100000



#define LCD_data_enable_pin_duration 3000
#define LCD_cmd_enable_pin_duration 37

class LCD_Setup {
    private:
        const uint8_t *rs_pin{ nullptr };
        const uint8_t *rw_pin{ nullptr };
        const uint8_t *en_pin{ nullptr };
        const uint8_t *LCD_data_length{ nullptr };
        const uint8_t data_length{ (*LCD_data_length) };
        bool usingTwoDisplayLines{ false };
        union pin_arrays_union {
            uint8_t DL8_pin_array[8];
            uint8_t DL4_pin_array[4];
        } pin_arrays;

        bool usingShiftRegister{ false };
        const bool *SR_bit_order_is_MSB{ nullptr };
        void (*SR_func_ptr)(byte){ nullptr };

        uint8_t current_cursor_position{ 1 };
        uint8_t character_limit_per_line{ 80 };
        bool isAlreadyOnLine2{ false };

        void toggle_enable_pin(const unsigned int delay);

        void set_rs_and_rw(const unsigned int rs_state, const unsigned int rw_state);

        void send_byte_to_LCD(byte byte_to_send, const uint_fast16_t enable_timing = LCD_data_enable_pin_duration, const uint8_t pin_array_size = 8);

        void send_nibbles_to_LCD(const byte nibble1, const byte nibble2, const uint_fast16_t enable_timing = LCD_data_enable_pin_duration);

        void send_nibbles_to_SR(const byte nibble1, const byte nibble2, const uint_fast16_t enable_timing = LCD_cmd_enable_pin_duration);

        void moveCursorPosition(uint8_t &display_line, uint8_t &block_number);

        void translate_and_send_to_LCD(byte decimal_representation);

        void moveDisplay(uint8_t &num_of_moves, const bool goRight = false);

        void toggleCursor(const uint8_t state, bool isBlinking = false);

        void display() {}

        void display_to_LCD(double user_num);

        template<typename STR_PTR_TYPE>
        void display_to_LCD(STR_PTR_TYPE *user_str);

        template<typename NUM_TYPE>
        void display_to_LCD(NUM_TYPE user_num);

    public:

        LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, const uint8_t d7pin, const uint8_t d6pin, const uint8_t d5pin, const uint8_t d4pin, const uint8_t d3pin = 0, const uint8_t d2pin = 0, const uint8_t d1pin = 0, const uint8_t d0pin = 0);

        LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, void (*SR_func)(byte) = nullptr, const bool SR_is_MSB = true);


        // Writes characters to the screen
        template <typename STR_TYPE, typename... PACK_ARGS>
        void display(STR_TYPE *parameter, PACK_ARGS... parameter_pack) {
            display_to_LCD(parameter);
            display(parameter_pack...);
        }
        template <typename NON_STR_TYPE, typename... PACK_ARGS>
        void display(NON_STR_TYPE parameter, PACK_ARGS... parameter_pack) {
            display_to_LCD(parameter);
            display(parameter_pack...);
        }

        // Moves the screen to the left
        void moveDisplayLeft(uint8_t num_of_moves = 1);

        // Moves the screen to the right 
        void moveDisplayRight(uint8_t num_of_moves = 1);

        // Makes cursor visible
        void cursorON(const bool withBlink = false);

        // Hides cursor
        void cursorOFF();

        // Moves the cursor and the screen back to the starting position  
        void resetPositions();

        // Moves the cursor at the specified block on the given display line
        void moveCursor(uint8_t display_line, uint8_t block_number);

        // Deletes all the characters on the screen, including the cursor
        void clearDisplay();
};



/*  TEMPLATE DEFINITIONS  */

template <typename STR_PTR_TYPE>
void LCD_Setup::display_to_LCD(STR_PTR_TYPE *user_str) {
    set_rs_and_rw(HIGH, LOW);

    for (size_t i{ 0 }; user_str[i] != '\0'; i++) {
        uint_fast8_t current_char_in_decimal{ (uint_fast8_t)user_str[i] };

        translate_and_send_to_LCD(current_char_in_decimal);
    }
}


template <typename NUM_TYPE>
void LCD_Setup::display_to_LCD(NUM_TYPE user_num) {
    set_rs_and_rw(HIGH, LOW);

    uint_fast8_t highest_power_of_10{ 0 };
    NUM_TYPE temporary_copy{ user_num };

    while (temporary_copy / 10 != 0) {
        temporary_copy /= 10;
        highest_power_of_10++;
    }
    for (int i{ highest_power_of_10 }; i > -1; i--) {
        NUM_TYPE digit = ((NUM_TYPE)(user_num / pow(10, i))) % 10;
        translate_and_send_to_LCD(digit + 48);
    }
}

#endif