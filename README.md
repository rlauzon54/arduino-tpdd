# arduino-tpdd
An Arduino-based system that emulates a Tandy Portable Disk Drive for the TRS-80 Model 100 and 102

# Project status: Failure

The problem part is the RS-232 communications.

The communications works with my test programs.  I am able to send the command and receive the correct response.  But when I try using the TS-DOS or TEENY, the T102 doesn't respond the same.

I started out by assuming that the dlplus was "correct" and emulated what it sent for the responses.  I wrote programs that sent commands and checked the responses, comparing them to what dlplus would do.

I got communications to work and I compared the input/output using the dlplus software with the input/output of my project.

For each command input, I responded with the same data.  But after a short period, the commands go different for my Arduino.

The TPDD and T102 need hardware flow control.  My belief is that since I am not doing real hardware flow control, the T102 side says "stop", but my project keeps sending - resulting in dropped characters on the T102 side.  Basically, a garbled response.

I wrote a simple BASIC program that sends canned commands and validates against the known responses - based on what dlplus sends/receives (TEST1.DO).  That works.

I did go down a path by reducing the speed of the connection to 9600.  That seemed to give me more, but it was a red herring.  I looked through the [TS-DOS dump](http://bitchin100.com/wiki/index.php?title=M100_TS-DOS_ROM_TPDD_Protocol) (nicely annotated - thank you!) and I tried to go through the code to see what I was doing wrong.

I located the routine in TS-DOS that sends the first commands and checks to see if I'm using Desklink/NADSBox.  I see the M1 08 command.  It uses that to see if the drive responds at 19200 BPS.  If it doesn't, it assumes that it's NOT Desklink/NADSBox.  It then sends a M1 07 command.  I see the M1 07 come accross at 19200.  But when it checks it's receive buffer, there's no response, which makes it drop to 9600 BPS - and that's the end of comminucations.

At this point, I don't know what the issue is.  I see the Arduino sending the correct data for the M1 07 command (it matches what dlplus sends).

In TS-DOS 4A94H, it calls 49A2H which sends the M1 08, M1 07 commands.  It then checks the queue for pending characters.  If it finds none, it lowers the speed to 9600.

For dlplus, 4A94H works, which returns to 4943H, which calls 49A2H again.  I see this in the output.  Then the 08 commands is sent.  Again, I see this for dlplus.

But for the Arduino, only the first call to 49A2H happens, then the speed is set to 9600 and communications fails.

Yet, when I write the BASIC program that sends the same commands, the Arduino responds with the right values - everything it send/received with no problem.  So I know that communication works.

I tried delaying the response for each command.
I tried a delay between each byte sent.
Arduino SoftwareSerial does NOT buffer out bound data.

At this point, I've run out of ideas.  I can't see why TS-DOS doesn't see the response for the 07 command.  My only theory goes back to the hardware flow control, which I can't seem to get anything for in Arduino-land.

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

