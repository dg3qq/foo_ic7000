# emulate some CI-V properties of a _ICOM IC-7000_ radio

This sketch/project implements a _Icom_ CI-V interface and acts like a _IC-7000_ radio in concern of channel management.

## Why ?

I needed a tool to debug some device driver of the CHIRP project, especially the 'ICOM' CI-V live radio types like IC-7000.
Using a real radio is not the best thing to do, isn't it.

## Hardware required

### Shopping cart:

* ARUINO micro (why? see below) 
* TTL/USB-Serial adapter
* bread-board and jumper wires
* (optional) a USB-Hub (a passive type will do)  to conserve USB-Ports on Your PC/Laptop and have things tidy 
    
    
### Why this is desingned to use a _ARUINO_ _micro_

Well, because at least two serial ports are needed:  

* 1-st to act as the CI-V port to emulated the radio
* 2-nd as debug and command terminal as well as to flash the _ARDUINO_ 

And I am not very fond of using any serial port (USART) emulation in the first place.

A _ARUINO_ _mega_ or _Teensy_ > 3.xx will also do, but these are currently not supported by the source code.

## Schematic

The wiring is streigt foward;

* connect TxD of ARDUINO micro to USB-Serial RxD input 
* connect RxD of ARDUINO micro to USB-Serial TxD output
* connect both GND to each other
    
## About Limitations

The ARDUINO micro has not enough memory (RAM/EEPROM) to provide for all five banks of 100 channel each a real 
IC7000 radio would have. To accomadate for that just two banks with only ten channals each are implemented. 
Not implemented channal memories will just return 'empty' on read request and silently ignore when written to.
         
## ToDo

Consider this to be still work in progress. There are still some soar edges. 

What comes to mind:

* (__most important__) create description (OP-manual) of command line interface
* check for string and buffer overflow
* refactor code base for better reusability and clarity  
* tidy up and improve source code documentation
* draw schematic diagram (fritzing?)
* think on porting to beefy hardware (ESP32)

## version history
*  20211203 - initial GitHub commit
*  20211204 - fixed #1 - misconception in response to access on empty/blank memory 
    + radio should respond with 'seperate' style telegram (see icom-civ manual; section 5-3)

## About links/references to 3-rd party documents or websites
+  I reject __any__ resopnsibility about what other sites will do.
    + _the internet_ works https://en.wikipedia.org/wiki/Tim_Berners-Lee


