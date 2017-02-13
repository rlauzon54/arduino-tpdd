# arduino-tpdd
An Arduino-based system that emulates a Tandy Portable Disk Drive for the TRS-80 Model 100 and 102

Hardware:
* Arduino Mega
* [RS-232 Shifter](https://www.sparkfun.com/products/449)
* [MicroSD shield](https://www.sparkfun.com/products/12761)

The interface on my T102 is an RS-232 serial port.  So the shifter is needed to translate RS-232 to TTL on the Arduino.

Much of the code is based on a [DeskLink port to Linux](http://www.bitchin100.com/).
The protocol is (reverse engineered) documented [here](http://bitchin100.com/wiki/index.php?title=TPDD_Base_Protocol)

Current status:
* Serial communications works.
* SD card works

Failures:
The Arduino Uno R3 does not have enough SRAM (only 2K).  Once the SD library is loaded, there's no room for anything else.
Moving to an Arduino Mega which has 8K of SRAM.
