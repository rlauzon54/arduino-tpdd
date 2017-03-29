# arduino-tpdd
An Arduino-based system that emulates a Tandy Portable Disk Drive for the TRS-80 Model 100 and 102

# Project status: Failure (so far)

The problem part is the RS-232 communications.

The communications works with my test programs.  I am able to send the command and receive the correct response.  But when I try using the TS-DOS or TEENY program, the T102 doesn't respond correctly.

I started out by assuming that the dlplus was "correct" and emulated what it sent for the responses.  I wrote programs that sent commands and checked the responses, comparing them to what dlplus would do.

I got communications to work and I compared the input/output using the dlplus software with the input/output of my project.
For each command input, I responded with the same data.  But the command/response is different.

I hooked both dlplus and my project up.  The only thing that I did was get a directory list with 1 file in the directory.

The first command is:
4D 31 0D 5A 5A 08 00 F7
Both dlplus and my project respond with
12 0B 00 52 4F 4F 54 20    20 2E 3C 3E 20 96

The second command is:
4D 31 0D 5A 5A 07 00 F8
Both respond with
12 01 00 EC

The third command is
4D 31 0D 5A 5A 08 00 F7 - using dlplus
But only
5A 5A 08 00 F7 - using my project
Both respond wtih
12 0B 00 52 4F 4F 54 20    20 2E 3C 3E 20 96

After that, it diverges.  My project gets
Mystery command 2
Mystery command 1
Mystery command 1
Mystery command 1

dlplus gets:
4D 31 0D 5A 5A 07 00 F8
5A 5A 08 00 F7

Finally, they both get:
5A 5A 00 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 46 01 9E
And both respond with
11 1C 41 44 56 45 4E 31    2E 42 41 20 20 20 20 20
20 20 20 20 20 20 20 20    20 20 46 45 D3 28 1C

But my project gets an error while dlplus shows the file in the directory.

The TPDD and T102 need hardware flow control.  My belief is that since I am not doing real hardware flow control, the T102 side says "stop", but my project keeps sending - resulting in dropped characters on the T102 side.  Basically, a garbled response.

I'm still trying.  My testing not using TS-DOS/TEENY seems to be working, though.  Still testing.

# Hardware explanation

Our base is a microcontroller.  I needed more memory than the Arduino Uno, so I went with the Arduino Mega to get enough SRAM.  The SD shield plus SoftwareSerial libraries just wouldn't fit in the Uno's small memory.

The microSD shield was a no-brainer.  The only thing with that is that I had to solder the SPI headers on since the microSD shield was made for the Uno and the usual pin outs were not compatible with the Mega.  The SPI header, though, lined up.

The biggest pain was the RS-232 shifter.  RS-232 operates from 3.3V to 12V.  So we need something to 1. shift DOWN the voltage (because sending more than 5V down the Arduino's TTL lines would fry it) and 2. to shift UP the TTL voltage to something that my T102 would like to see.

The problem is most shifters are not made to be full, bi-directional RS-232.  I've tried:

* [RS-232 Shifter](https://www.sparkfun.com/products/449)
The first one I tried only wired up the TX, RX and GND lines.  That failed because the T102 uses hardware flow control.  I tried modifying the cable to loop back the control lines, but that resulted in dropped characters on the T102 side.

* [RS232 to TTL converter board DTE with male DB9 3.3V to 5V](https://www.amazon.com/gp/product/B0088SNIOQ/ref=oh_aui_detailpage_o03_s01?ie=UTF8&psc=1)
The second one I tried had the RTS/CTS control lines, but they were uni-directional (the Arduino side could not assert RTS, for example, only read it).

* [RS232 to 5V TTL Converter](http://www.serialcomm.com/serial_rs232_converters/rs232_rs485_to_ttl_converters/rs232_to_5v_ttl_converter/rs232_to_5v_ttl.product_general_info.aspx)
This one promises to be a full bi-directional port.  And it is, but it loops back the control lines on the RS-232 side, so no real flow control.

Hardware:
* Arduino Mega
[MicroSD Shield](https://www.sparkfun.com/products/12761)

Much of the code is based on a [DeskLink port to Linux](http://www.bitchin100.com/).
The protocol is (reverse engineered) documented [here](http://bitchin100.com/wiki/index.php?title=TPDD_Base_Protocol)

Arduino pins used:
* 7 - Drive activity light
* Digital ground - Drive activity light (gnd)
* 62 (A8) - Shifter TX
* 63 (A9) - Shifter RX
* Analog 5V and ground for the power for the shifter.
* SPI header for SD shield

Unavailable pins:
* 10-13
* 50-53
* 8 (card select)

Notes:
The Mega puts the SPI pins on 50-53 instead of 10-13.  So you need to solder the SPI header to the MicroSD Shield in order to move the pins.  As a result, pins 50-53 on the Mega are unavailable.  Also, because of how the MicroSD Shield is made, pins 10-13 are unavailable as well - if you use the stacking headers.  You can probably get around this by not using the stacking headers for pins D8-D13 and connecting stuff directly to the Mega and not through the MicroSD Shield.

Not all pins on the Mega and Mega 2560 support change interrupts, so only the following can be used for RX:
10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69
So, since 10-13 and 50-53 are unavailable, I had to put the shifter on pins 62 and 63.

The cable used to connect the T102 to the Arduino must be "special".
On the Club 100 web site, there is instructions on how to [create a null modem cable](http://www.club100.org/library/doc/cables.html).
The shifter doesn't use anything but the TX, RX, and Ground pins.  So you really only need to loop back the DB-25 (T102) end of the cable.  The RTS/CTS and DTR/DSR pins should be looped back to each other (pins 4-5 and 6-20).  This provides the feedback the T102 side needs for communications.
The "special"ness of the cable that is needed for this project DOES NOT CROSS THE TX and RX lines.  Those need to go straight through.  TX<->TX and RX<->RX.  The null modem instructions would have you TX<->RX and RX<->TX.  Don't do that.

