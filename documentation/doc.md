## .display( *data...* )
**Parameter(s):**\
char, string, uint, float/double

**Description:**\
Translates data to ASCII characters and displays them to the LCD. Note: for negative numbers, you will have to use a workaround, i.e. "-".

**Syntax:**
```cpp
mylcd.display("My favourite number is ", 2);

mylcd.display(3.14, " < ", 3.141);
```
<br></br>
## .moveDisplayLeft( *num_of_moves* ) OR .moveDisplayRight( *num_of_moves* )
**Parameter(s):**\
uint

**Description:**\
Moves the entire screen to the left or right; the amount it moves the screen by depends on the value you pass in. 

**Syntax:**
```cpp
mylcd.moveDisplayLeft(); // both functions default to 1 move when there's no argument passed to them

mylcd.moveDisplayRight(10);
```
<br></br>
## .cursorON( *withBlink* )
**Parameter(s):**\
bool

**Description:**\
Displays the LCD's built-in cursor. 

**Syntax:**
```cpp
mylcd.cursorON(); // no blink (displayed as an underscore)

mylcd.cursorON(true); // with blinking bar
```
<br></br>
## .cursorOFF( )
**Description:**\
Hides the LCD's built-in cursor. 

**Syntax:**
```cpp
mylcd.cursorOFF();
```
<br></br>
## .resetPositions( )
**Description:**\
Resets any movements made to the LCD screen and moves the cursor back to the first character. 

**Syntax:**
```cpp
mylcd.resetPositions();
```
<br></br>
## .moveCursor( *display_line*, *block_number* )
**Parameter(s):**\
uint

**Description:**\
Displays the LCD's built-in cursor. 

**Syntax:**
```cpp
/* 

The Hitachi LCDs that are commonly used for Arduino projects typically only use 2 display lines at a time - even the 20x4 models; 
That is why my function will only allow you to choose between display line 1 or 2. If you go over or under these numbers, it 
will be corrected to the nearest one.

Note:
You can write up to 80 characters to the LCD in total. If your LCD is set to two display lines, this total will be halved to 40
per line. This total determines the limit of your column value per line.

*/

mylcd.moveCursor(1, 16); // row 1 (i.e. first display line), column 16 (i.e. 16th character block) 

mylcd.moveCursor(2, 1); // row 2 (i.e. second display line), column 1 (i.e. 1st character block)
```
<br></br>
## .clearDisplay( )
**Description:**\
Deletes all the characters that were written to the LCD, including the cursor. 

**Syntax:**
```cpp
mylcd.clearDisplay();
```
