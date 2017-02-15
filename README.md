# arduino-tpdd
An Arduino-based system that emulates a Tandy Portable Disk Drive for the TRS-80 Model 100 and 102

Hardware:
* Arduino Mega
* [RS-232 Shifter](https://www.sparkfun.com/products/449)
* [MicroSD Shield](https://www.sparkfun.com/products/12761)

The interface on the T102 is an RS-232 serial port.  So the shifter is needed to translate RS-232 to TTL on the Arduino.

Much of the code is based on a [DeskLink port to Linux](http://www.bitchin100.com/).
The protocol is (reverse engineered) documented [here](http://bitchin100.com/wiki/index.php?title=TPDD_Base_Protocol)

Pins used:
* 7 - Drive activity light
* Digital ground - Drive activity light
* 62 (A8) - Shifter TX
* 63 (A9) - Shifter RX
* Analog 5V and ground for the power for the shifter.
* SPI header for SD shield

Unavailable pins:
* 10-13
* 50-53
* 8 (card select)

Current status:
* Serial communications and SD card work together.

Notes:
The Mega puts the SPI pins on 50-53 instead of 10-13.  So you need to solder the SPI header to the MicroSD Shield in order to move the pins.  As a result, pins 50-53 on the Mega are unavailable.  Also, because of how the MicroSD Shield is made, pins 10-13 are unavailable as well - if you use the stacking headers.  You can probably get around this by not using the stacking headers for pins D8-D13 and connecting stuff directly to the Mega and not through the MicroSD Shield.

Not all pins on the Mega and Mega 2560 support change interrupts, so only the following can be used for RX:
10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69
So, since 10-13 and 50-53 are unavailable, I had to put the shifter on pins 62 and 63. 