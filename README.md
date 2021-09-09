# LCD++ (Arduino LCD library)
This is my own custom library for controlling LCDs with the *Arduino*. It provides commonly used features that are available in the *LiquidCrystal Library* but it adds the ability to connect the LCD's data pins to an 8-pin shift register like the 74HC595.
<br></br>
## Features
- Control the data pins of your LCD with a shift register (I've only tested it with the 74HC595, but I think any 8-pin one will do).
- Write a combination of strings, integers, or even floats to the display using one function.
- More descriptive functions and parameters.
- The setting up is done with the class constructor only, which may help your code look more readable.
