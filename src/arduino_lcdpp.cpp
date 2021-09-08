#include <Arduino.h>
#include <arduino_lcdpp.h>



LCD_Setup::LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, const uint8_t d7pin, const uint8_t d6pin, const uint8_t d5pin, const uint8_t d4pin, const uint8_t d3pin, const uint8_t d2pin, const uint8_t d1pin, const uint8_t d0pin) {
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


LCD_Setup::LCD_Setup(const uint8_t rs, const uint8_t rw, const uint8_t en, const uint8_t data_length, const bool has2DisplayLines, const bool has5x10Font, void (*SR_func)(byte), const bool SR_is_MSB) {
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


void LCD_Setup::moveDisplayLeft(uint8_t num_of_moves) {
    moveDisplay(num_of_moves);
}


void LCD_Setup::moveDisplayRight(uint8_t num_of_moves) {
    moveDisplay(num_of_moves, true);
}


void LCD_Setup::cursorON(const bool withBlink) {
    toggleCursor(1, withBlink);
}


void LCD_Setup::cursorOFF() {
    toggleCursor(0);
}


void LCD_Setup::resetPositions() {
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


void LCD_Setup::moveCursor(uint8_t display_line, uint8_t block_number) {
    moveCursorPosition(display_line, block_number);
}


void LCD_Setup::clearDisplay() {
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
}
