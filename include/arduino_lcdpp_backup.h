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

        LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, const uint8_t d7pin, const uint8_t d6pin, const uint8_t d5pin, const uint8_t d4pin, const uint8_t d3pin = 0, const uint8_t d2pin = 0, const uint8_t d1pin = 0, const uint8_t d0pin = 0) {
            LCD_data_length = &data_length;
            usingTwoDisplayLines = has2DisplayLines;
            if (has2DisplayLines) { character_limit_per_line = 40; }

            switch (data_length) {
                case 8:
                    pin_arrays.DL8_pin_array[0] = d7pin;
                    pin_arrays.DL8_pin_array[1] = d6pin;
                    pin_arrays.DL8_pin_array[2] = d5pin;
                    pin_arrays.DL8_pin_array[3] = d4pin;
                    pin_arrays.DL8_pin_array[4] = d3pin;
                    pin_arrays.DL8_pin_array[5] = d2pin;
                    pin_arrays.DL8_pin_array[6] = d1pin;
                    pin_arrays.DL8_pin_array[7] = d0pin;

                    for (size_t i{ 0 }; i < 8; i++) {
                        pinMode(pin_arrays.DL8_pin_array[i], OUTPUT);
                    }
                    break;

                default:
                    pin_arrays.DL4_pin_array[0] = d7pin;
                    pin_arrays.DL4_pin_array[1] = d6pin;
                    pin_arrays.DL4_pin_array[2] = d5pin;
                    pin_arrays.DL4_pin_array[3] = d4pin;

                    for (size_t i{ 0 }; i < 4; i++) {
                        pinMode(pin_arrays.DL4_pin_array[i], OUTPUT);
                    }
            }

            rs_pin = &rs;
            rw_pin = &rw;
            en_pin = &en;
            pinMode(rs, OUTPUT);
            pinMode(rw, OUTPUT);
            pinMode(en, OUTPUT);

            delayMicroseconds(45000); // delay of more than 40 milliseconds which allows the LCD power to rise to 2.7 volts, the minimum power required to initialize the LCD by instruction, as specified on pg45-46 of the Hitachi HD44780 datasheet. Since the Arduino doesn't meet the required power supply conditions (4.5 volts) to activate the internal reset circuit of the LCD automatically, this has to be done.

            set_rs_and_rw(LOW, LOW);
            byte displayLines_and_font;

            // INITIALIZATION BY INSTRUCTION
            switch (data_length) {
                case 8:
                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration);
                    delayMicroseconds(4500);

                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration);
                    delayMicroseconds(150);

                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration);


                    displayLines_and_font = funcset_DL8_MSB;
                    if (has2DisplayLines) { displayLines_and_font |= B00010000; }
                    if (has5x10Font) { displayLines_and_font |= B00100000; }

                    send_byte_to_LCD(displayLines_and_font, LCD_cmd_enable_pin_duration);

                    send_byte_to_LCD(clearLCD_DL8_MSB, LCD_cmd_enable_pin_duration);

                    send_byte_to_LCD(returnLCDHome_DL8_MSB, 1520);

                    send_byte_to_LCD(entryset_incr_shift0_DL8_MSB, LCD_cmd_enable_pin_duration);

                    send_byte_to_LCD(display1_cursor0_blink0_DL8_MSB, LCD_cmd_enable_pin_duration);

                    break;

                default:
                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration, 4);
                    delayMicroseconds(4500);

                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration, 4);
                    delayMicroseconds(150);

                    // temporary function set (8 bit data length)
                    send_byte_to_LCD(funcset_DL8_MSB, LCD_cmd_enable_pin_duration, 4);

                    // temporary function set (4 bit data length)
                    send_byte_to_LCD(funcset_DL4_MSB, LCD_cmd_enable_pin_duration, 4);


                    displayLines_and_font = B00000000;
                    if (has2DisplayLines) { displayLines_and_font |= B00000001; }
                    if (has5x10Font) { displayLines_and_font |= B00000010; }

                    send_nibbles_to_LCD(B00000100, displayLines_and_font, LCD_cmd_enable_pin_duration);

                    send_nibbles_to_LCD(B00000000, clearLCD_DL4_MSB, LCD_cmd_enable_pin_duration);

                    send_nibbles_to_LCD(B00000000, returnLCDHome_DL4_MSB, 1520);

                    send_nibbles_to_LCD(B00000000, entryset_incr_shift0_DL4_MSB, LCD_cmd_enable_pin_duration);

                    send_nibbles_to_LCD(B00000000, display1_cursor0_blink0_DL4_MSB, LCD_cmd_enable_pin_duration);
            }

            delayMicroseconds(100);
        }


        LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, void (*SR_func)(byte) = nullptr, const bool SR_is_MSB = true) {
            LCD_data_length = &data_length;
            usingTwoDisplayLines = has2DisplayLines;
            if (has2DisplayLines) { character_limit_per_line = 40; }

            usingShiftRegister = true;
            SR_bit_order_is_MSB = &SR_is_MSB;
            SR_func_ptr = SR_func;

            rs_pin = &rs;
            rw_pin = &rw;
            en_pin = &en;
            pinMode(rs, OUTPUT);
            pinMode(rw, OUTPUT);
            pinMode(en, OUTPUT);

            delayMicroseconds(45000); // delay of more than 40 milliseconds which allows the LCD power to rise to 2.7 volts, the minimum power required to initialize the LCD by instruction, as specified on pg45-46 of the Hitachi HD44780 datasheet. Since the Arduino doesn't meet the required power supply conditions (4.5 volts) to activate the internal reset circuit of the LCD automatically, this has to be done.

            set_rs_and_rw(LOW, LOW);
            byte displayLines_and_font;

            // INITIALIZATION BY INSTRUCTION
            switch (data_length) {
                case 8:
                    displayLines_and_font = B00000000;
                    SR_is_MSB ? displayLines_and_font |= B00001100 : displayLines_and_font |= B00110000;

                    if (has2DisplayLines) {
                        SR_is_MSB ? displayLines_and_font |= B00010000 : displayLines_and_font |= B00001000;
                    }
                    if (has5x10Font) {
                        SR_is_MSB ? displayLines_and_font |= B00100000 : displayLines_and_font |= B00000100;
                    }

                    if (SR_is_MSB) {
                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(4500);

                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(150);

                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(displayLines_and_font);
                        toggle_enable_pin(37);

                        SR_func_ptr(clearLCD_DL8_MSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(returnLCDHome_DL8_MSB);
                        toggle_enable_pin(1520);

                        SR_func_ptr(entryset_incr_shift0_DL8_MSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(display1_cursor0_blink0_DL8_MSB);
                        toggle_enable_pin(37);
                    }
                    else {
                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(4500);

                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(150);

                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(displayLines_and_font);
                        toggle_enable_pin(37);

                        SR_func_ptr(clearLCD_DL8_LSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(returnLCDHome_DL8_LSB);
                        toggle_enable_pin(1520);

                        SR_func_ptr(entryset_incr_shift0_DL8_LSB);
                        toggle_enable_pin(37);

                        SR_func_ptr(display1_cursor0_blink0_DL8_LSB);
                        toggle_enable_pin(37);
                    }

                    break;

                default:
                    displayLines_and_font = B00000000;
                    if (has2DisplayLines) {
                        SR_is_MSB ? displayLines_and_font |= B00000001 : displayLines_and_font |= B10000000;
                    }
                    if (has5x10Font) {
                        SR_is_MSB ? displayLines_and_font |= B00000010 : displayLines_and_font |= B01000000;
                    }

                    if (SR_is_MSB) {
                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(4500);

                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(150);

                        SR_func_ptr(funcset_DL8_MSB);
                        toggle_enable_pin(37);

                        // temporary function set (4 bit data length)
                        SR_func_ptr(funcset_DL4_MSB);
                        toggle_enable_pin(37);

                        send_nibbles_to_SR(B00000100, displayLines_and_font);

                        send_nibbles_to_SR(B00000000, clearLCD_DL4_MSB);

                        send_nibbles_to_SR(B00000000, returnLCDHome_DL4_MSB, 1520);

                        send_nibbles_to_SR(B00000000, entryset_incr_shift0_DL4_MSB);

                        send_nibbles_to_SR(B00000000, display1_cursor0_blink0_DL4_MSB);
                    }
                    else {
                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(4500);

                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);
                        delayMicroseconds(150);

                        SR_func_ptr(funcset_DL8_LSB);
                        toggle_enable_pin(37);

                        // temporary function set (4 bit data length)
                        SR_func_ptr(funcset_DL4_LSB);
                        toggle_enable_pin(37);

                        send_nibbles_to_SR(B00000100, displayLines_and_font);

                        send_nibbles_to_SR(B00000000, clearLCD_DL4_LSB);

                        send_nibbles_to_SR(B00000000, returnLCDHome_DL4_LSB, 1520);

                        send_nibbles_to_SR(B00000000, entryset_incr_shift0_DL4_LSB);

                        send_nibbles_to_SR(B00000000, display1_cursor0_blink0_DL4_LSB);
                    }
            }
        }



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
        void moveDisplayLeft(uint8_t num_of_moves = 1) {
            moveDisplay(num_of_moves);
        }


        // Moves the screen to the right 
        void moveDisplayRight(uint8_t num_of_moves = 1) {
            moveDisplay(num_of_moves, true);
        }


        // Makes cursor visible
        void cursorON(const bool withBlink = false) {
            toggleCursor(1, withBlink);
        }


        // Hides cursor
        void cursorOFF() {
            toggleCursor(0);
        }


        // Moves the cursor and the screen back to the starting position  
        void resetPositions() {
            set_rs_and_rw(LOW, LOW);
            const uint8_t data_length{ (*LCD_data_length) };
            const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };

            if (usingShiftRegister) {
                switch (data_length) {
                    case 8:
                        SR_is_MSB ? SR_func_ptr(B01000000) : SR_func_ptr(B00000010);
                        break;

                    default:
                        SR_is_MSB ? send_nibbles_to_SR(B00000000, B00000100) : send_nibbles_to_SR(B00000000, B00100000);
                }
            }
            else {
                delayMicroseconds(100);

                switch (data_length) {
                    case 8:
                        send_byte_to_LCD(B01000000, 1520);
                        break;

                    default:
                        send_nibbles_to_LCD(B00000000, B00000100, 1520);
                }
            }
        }


        // Moves the cursor at the specified block on the given display line
        void moveCursor(uint8_t display_line, uint8_t block_number) {
            moveCursorPosition(display_line, block_number);
        }


        // Deletes all the characters on the screen, including the cursor
        void clearDisplay() {
            set_rs_and_rw(LOW, LOW);
            const uint8_t data_length{ (*LCD_data_length) };
            const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };

            if (usingShiftRegister) {
                switch (data_length) {
                    case 8:
                        SR_is_MSB ? SR_func_ptr(B10000000) : SR_func_ptr(B00000001);
                        break;

                    default:
                        SR_is_MSB ? send_nibbles_to_SR(B00000000, B00001000) : send_nibbles_to_SR(B00000000, B00010000);
                }
            }
            else {
                delayMicroseconds(100);

                switch (data_length) {
                    case 8:
                        send_byte_to_LCD(B10000000, LCD_cmd_enable_pin_duration);
                        break;

                    default:
                        send_nibbles_to_LCD(B00000000, B00001000, LCD_cmd_enable_pin_duration);
                }
            }
        }
};






/*  BITWISE MANIPULATION  */

void LCD_Setup::toggle_enable_pin(const unsigned int delay) {
    digitalWrite((*en_pin), HIGH);
    delayMicroseconds(delay);
    digitalWrite((*en_pin), LOW);
}



void LCD_Setup::set_rs_and_rw(const unsigned int rs_state, const unsigned int rw_state) {
    digitalWrite((*rs_pin), rs_state);
    digitalWrite((*rw_pin), rw_state);
}



void LCD_Setup::send_byte_to_LCD(byte byte_to_send, const uint_fast16_t enable_timing, const uint8_t pin_array_size) {
    for (size_t i{ 0 }; i < pin_array_size; i++) {
        digitalWrite(pin_arrays.DL8_pin_array[i], (byte_to_send & B00000001));
        byte_to_send >>= 1;
    }

    toggle_enable_pin(enable_timing);
}



void LCD_Setup::send_nibbles_to_LCD(const byte nibble1, const byte nibble2, const uint_fast16_t enable_timing) {
    byte current_nibble{ nibble1 };

    for (size_t i{ 0 }; i < 2; i++) {
        for (size_t j{ 0 }; j < 4; j++) {
            digitalWrite(pin_arrays.DL4_pin_array[j], (current_nibble & B00000001));
            current_nibble >>= 1;
        }

        current_nibble = nibble2;
        toggle_enable_pin(enable_timing);
    }
}



void LCD_Setup::send_nibbles_to_SR(const byte nibble1, const byte nibble2, const uint_fast16_t enable_timing) {
    SR_func_ptr(nibble1);
    toggle_enable_pin(enable_timing);
    SR_func_ptr(nibble2);
    toggle_enable_pin(enable_timing);
}



void LCD_Setup::translate_and_send_to_LCD(byte decimal_representation) {
    byte byte_to_send{ decimal_representation }, nibble1{ B00000000 }, nibble2{ B00000000 }, *current_nibble{ &nibble2 }, byte_to_OR_with;
    bool is_a_hit{ false };

    delayMicroseconds(1000);

    if (usingShiftRegister) {
        const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };

        switch (data_length) {
            case 8:
                break;

            default:
                for (size_t i{ 0 }; i < 2; i++) {
                    SR_is_MSB ? byte_to_OR_with = B00001000 : byte_to_OR_with = B00010000;

                    for (size_t j{ 0 }; j < 4; j++) {
                        is_a_hit = B00000001 == (decimal_representation & B00000001);
                        decimal_representation >>= 1;

                        is_a_hit ? (*current_nibble) |= byte_to_OR_with : (*current_nibble) |= B00000000;
                        SR_is_MSB ? byte_to_OR_with >>= 1 : byte_to_OR_with <<= 1;
                    }

                    current_nibble = &nibble1;
                }

                send_nibbles_to_SR(nibble1, nibble2, 3000);
        }
    }
    else {
        switch (data_length) {
            case 8:
                byte_to_send = B00000000;
                byte_to_OR_with = B00000001;

                for (size_t i{ 0 }; i < 8; i++) {
                    is_a_hit = B00000001 == (decimal_representation & B00000001);
                    decimal_representation >>= 1;

                    is_a_hit ? byte_to_send |= byte_to_OR_with : byte_to_send |= B00000000;
                    byte_to_send <<= 1;
                }

                send_byte_to_LCD(byte_to_send); 

                break;

            default:
                for (size_t i{ 0 }; i < 2; i++) {
                    byte_to_OR_with = B00001000;

                    for (size_t j{ 0 }; j < 4; j++) {
                        is_a_hit = B00000001 == (decimal_representation & B00000001);
                        decimal_representation >>= 1;

                        is_a_hit ? (*current_nibble) |= byte_to_OR_with : (*current_nibble) |= B00000000;
                        byte_to_OR_with >>= 1;
                    }

                    current_nibble = &nibble1;
                }

                send_nibbles_to_LCD(nibble1, nibble2);
        } 
    }

    current_cursor_position++;
}




/*  LCD INTERFACING  */

void LCD_Setup::moveDisplay(uint8_t &num_of_moves, const bool goRight) {
    set_rs_and_rw(LOW, LOW);
    byte byte_to_send, nibble2;

    if (usingShiftRegister) {
        const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };

        switch (data_length) {
            case 8:
                if (SR_is_MSB) {
                    goRight ? byte_to_send = B00011000 : byte_to_send = B00111000;
                }
                else {
                    goRight ? byte_to_send =  B00011000 : byte_to_send = B00011100;
                }

                for (size_t i{ 0 }; i < num_of_moves; i++) {
                    SR_func_ptr(byte_to_send);
                }

                break;

            default:
                byte nibble1;

                SR_is_MSB ? nibble1 = B00001000 : nibble1 = B00010000;

                if (SR_is_MSB) {
                    goRight ? nibble2 = B00000001 : nibble2 = B00000011;
                }
                else {
                    goRight ? nibble2 = B10000000 : nibble2 = B11000000;
                }

                for (size_t i{ 0 }; i < num_of_moves; i++) {
                    send_nibbles_to_SR(nibble1, nibble2);
                }
        }
    }
    else {
        switch (data_length) {
            case 8:
                goRight ? byte_to_send = B00011000 : byte_to_send = B00111000;

                for (size_t i{ 0 }; i < num_of_moves; i++) {
                    send_byte_to_LCD(byte_to_send, LCD_cmd_enable_pin_duration);
                }

                break;

            default:
                goRight ? nibble2 = B00000001 : nibble2 = B00000011;

                for (size_t i{ 0 }; i < num_of_moves; i++) {
                    send_nibbles_to_LCD(B00001000, nibble2, LCD_cmd_enable_pin_duration);
                }
        }
    }
}



void LCD_Setup::toggleCursor(const uint8_t state, bool isBlinking) {
    set_rs_and_rw(LOW, LOW);
    byte nibble2;

    if (usingShiftRegister) {
        const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };
        delayMicroseconds(1000);

        switch (data_length) {
            case 8:
                if (state == 1) {
                    if (isBlinking) { 
                        SR_is_MSB ? SR_func_ptr(B11110000) : SR_func_ptr(B00001111);
                    }
                    else {
                        SR_is_MSB ? SR_func_ptr(B01110000) : SR_func_ptr(B00001110);
                    }
                }
                else {
                    SR_is_MSB ? SR_func_ptr(B00110000) : SR_func_ptr(B00001100);
                }

                break;

            default:
                if (state == 1) {
                    if (isBlinking) { 
                        SR_is_MSB ? send_nibbles_to_SR(B00000000, B00001111) : send_nibbles_to_SR(B00000000, B11110000);
                    }
                    else {
                        SR_is_MSB ? send_nibbles_to_SR(B00000000, B00000111) : send_nibbles_to_SR(B00000000, B11100000);
                    }
                }
                else {
                    SR_is_MSB ? send_nibbles_to_SR(B00000000, B00000011) : send_nibbles_to_SR(B00000000, B11000000);
                }
        }
    }
    else {
        delayMicroseconds(1000);

        switch (data_length) {
            case 8:
                if (state == 1) {
                    byte byte_to_send{ B01110000 };
                    if (isBlinking) { byte_to_send |= B10000000; }

                    send_byte_to_LCD(byte_to_send, LCD_cmd_enable_pin_duration);
                }
                else {
                    send_byte_to_LCD(B00110000, LCD_cmd_enable_pin_duration);
                }

                break;

            default:
                if (state == 1) {
                    nibble2 = B00000111;
                    if (isBlinking) { nibble2 |= B00001000; }

                    send_nibbles_to_LCD(B00000000, nibble2, LCD_cmd_enable_pin_duration);
                }
                else {
                    nibble2 = B00000011;
                    send_nibbles_to_LCD(B00000000, nibble2, LCD_cmd_enable_pin_duration);
                }
        }
    }
}



void LCD_Setup::moveCursorPosition(uint8_t &display_line, uint8_t &block_number) {
    set_rs_and_rw(LOW, LOW);
    const bool SR_is_MSB{ (*SR_bit_order_is_MSB) };

    // ensures that the cursor stays within the limits
    if (display_line < 1) { display_line = 1; } else if (display_line > 2) { display_line = 2; }
    if (block_number < 1) { block_number = 1; } else if (block_number > character_limit_per_line) { block_number = character_limit_per_line; }

    // moves the cursor to the next line only if the LCD has 2 display lines & is NOT on the 2nd line already
    if (usingTwoDisplayLines && display_line == 2 && !isAlreadyOnLine2) {
        isAlreadyOnLine2 = true;
        uint8_t current_position_alias{ current_cursor_position };

        if (!usingShiftRegister) {
            switch (data_length) {
                case 8:
                    for (; current_position_alias < 40; current_position_alias++) {
                        send_byte_to_LCD(B00101000, LCD_cmd_enable_pin_duration);
                    }
                    break;

                default:
                    for (; current_position_alias < 40; current_position_alias++) {
                        send_nibbles_to_LCD(B00001000, B00000010, LCD_cmd_enable_pin_duration);
                    }
            }
        }
        else {
            delayMicroseconds(1000);
            switch (data_length) {
                case 8:
                    if (SR_is_MSB) {
                        for (; current_position_alias < 40; current_position_alias++) {
                            SR_func_ptr(B00101000);
                        }
                    }
                    else {
                        for (; current_position_alias < 40; current_position_alias++) {
                            SR_func_ptr(B00010100);
                        }
                    }

                    break;

                default:
                    if (SR_is_MSB) {
                        for (; current_position_alias < 40; current_position_alias++) {
                            send_nibbles_to_SR(B00001000, B00000010);
                        }
                    }
                    else {
                        for (; current_position_alias < 40; current_position_alias++) {
                            send_nibbles_to_SR(B00010000, B01000000);
                        }
                    }
            }
        }
        current_cursor_position = 0; // after moving to 2nd display line, reset position tracker
    }

    // moves the cursor back to the first display line only if it was on the second
    else if (usingTwoDisplayLines && display_line == 1 && isAlreadyOnLine2) {
        isAlreadyOnLine2 = false;
        uint8_t current_position_alias = current_cursor_position + 40;

        if (!usingShiftRegister) {
            switch (data_length) {
                case 8:
                    for (; current_position_alias != current_cursor_position; current_position_alias--) {
                        send_byte_to_LCD(B00001000, LCD_cmd_enable_pin_duration);
                    }
                    break;

                default:
                    for (; current_position_alias != current_cursor_position; current_position_alias--) {
                        send_nibbles_to_LCD(B00001000, B00000000, LCD_cmd_enable_pin_duration);
                    }
            }
        }
        else {
            delayMicroseconds(1000);
            switch (data_length) {
                case 8:
                    if (SR_is_MSB) {
                        for (; current_position_alias != current_cursor_position; current_position_alias--) {
                            SR_func_ptr(B00001000);
                        }
                    }
                    else {
                        for (; current_position_alias != current_cursor_position; current_position_alias--) {
                            SR_func_ptr(B00010000);
                        }
                    }

                    break;

                default:
                    if (SR_is_MSB) {
                        for (; current_position_alias != current_cursor_position; current_position_alias--) {
                            send_nibbles_to_SR(B00001000, B00000000);
                        }
                    }
                    else {
                        for (; current_position_alias != current_cursor_position; current_position_alias--) {
                            send_nibbles_to_SR(B00010000, B00000000);
                        }
                    }
            }
        }
    }



    // this part moves the cursor along a display line
    if (!usingShiftRegister) {
        switch (data_length) {
            case 8:
                if (current_cursor_position > block_number) {
                    for (; current_cursor_position > block_number; current_cursor_position--) {
                        send_byte_to_LCD(B00101000, LCD_cmd_enable_pin_duration);
                    }
                }
                else {
                    for (; current_cursor_position < block_number; current_cursor_position++) {
                        send_byte_to_LCD(B00001000, LCD_cmd_enable_pin_duration);
                    }
                }

                break;

            default:
                if (current_cursor_position > block_number) {
                    for (; current_cursor_position > block_number; current_cursor_position--) {
                        send_nibbles_to_LCD(B00001000, B00000000, LCD_cmd_enable_pin_duration);
                    }
                }
                else {
                    for (; current_cursor_position < block_number; current_cursor_position++) {
                        send_nibbles_to_LCD(B00001000, B00000010, LCD_cmd_enable_pin_duration);
                    }
                }
        }
    }
    else {
        delayMicroseconds(1000);
        byte byte_to_send, nibble1, nibble2;

        switch (data_length) {
            case 8:
                if (current_cursor_position > block_number) {
                    SR_is_MSB ? byte_to_send = B00101000 : byte_to_send = B00010100;

                    for (; current_cursor_position > block_number; current_cursor_position--) {
                        SR_func_ptr(byte_to_send);
                    }
                }
                else {
                    SR_is_MSB ? byte_to_send = B00001000 : byte_to_send = B00010000;

                    for (; current_cursor_position < block_number; current_cursor_position++) {
                        SR_func_ptr(byte_to_send);
                    }
                }

                break;

            default:
                SR_is_MSB ? nibble1 = B00001000 : nibble1 = B00010000;

                if (current_cursor_position > block_number) {
                    for (; current_cursor_position > block_number; current_cursor_position--) {
                        send_nibbles_to_SR(nibble1, B00000000);
                    }
                }
                else {
                    SR_is_MSB ? nibble2 = B00000010 : nibble2 = B01000000;

                    for (; current_cursor_position < block_number; current_cursor_position++) {
                        send_nibbles_to_SR(nibble1, nibble2);
                    }
                }
        }
    }
}



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



void LCD_Setup::display_to_LCD(double user_num) {
    set_rs_and_rw(HIGH, LOW);
    uint_fast64_t integral_copy{ (uint_fast64_t)user_num };
    display_to_LCD(integral_copy);

    // the following section converts the part after the decimal point into a regular integer
    double fractional_copy = user_num - integral_copy;
    double temporary_copy{ fractional_copy };
    bool hasOverflown{ false };
    size_t  num_of_iterations{ 0 };

    while ((temporary_copy - (int)(temporary_copy)) <= 0.999) {
        temporary_copy *= 10;
        num_of_iterations++;

        if (num_of_iterations > 10) { hasOverflown = true; break; }
    }

    if (!hasOverflown) {
        for (size_t i{ 0 }; i < num_of_iterations; i++) {
            fractional_copy *= 10;
            if ((int)fractional_copy == 0) { display(0); }
        }

        display_to_LCD(".");
        display_to_LCD((uint_fast64_t)fractional_copy);
    }
};

#endif