![alt text](https://github.com/mistepien/snespad2usb/blob/main/top.svg)
![alt text](https://github.com/mistepien/snespad2usb/blob/main/bottom.svg)

# snespad2usb
SNES gampepad adapter to USB.

<a href="https://github.com/mistepien/snespad2usb/blob/main/snespad2usb.pdf">Adapter</a> is actually a shield for 
<a href="https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/hardware-overview-pro-micro">Arduino Pro Micro based on ATmega 32U4</a>.

ATmega 32U4 has a hardware support for fullspeed USB, thus <a href="https://wiki.archlinux.org/title/mouse_polling_rate">polling rate 1000Hz</a> is not an issue for that chip.

You can use the <a href="https://github.com/mistepien/segapad2usb/tree/main/production">gerber file<a> to order the PCB.

Attached <a href="https://github.com/mistepien/snespad2usb/tree/main/firmware">code</a> uses port registers so that is quite efficient. The code was tested with <a href="https://www.8bitdo.com/sn30-wireless-for-original-snes-sfc/">SN30 2.4g wireless gamepad for original SNES/SFC</a> and original SNES and SFC gamepads. Everything works flawless!

One footprint is not trivial: SNES gamapad socket (<b>J1</b>). Footprint is DIY.

<b>J2</b> is redundant -- you can use it to add some new features like autofire switch or sth.

Sometimes during uploading code to Arduino board you may face some issues -- then you will need to use RESET -- so <b>SW1</b> can be useful if you like to develope your own code.

SW2 can be usefull to have a so-called "Hot key" -- you can need it eg. in Batocera.

LED D1 shows is SNES gamepad connected at all. LED D2 is just a status LED (turn on in setup() function of firmware).

BOM:
| Qty	| Reference(s) | Description |
|-----|--------------|-------------|
|2 | D1, D2 | LEDs 3mm |  
|1 | J1| SNES gamepad make connector |
|1 | J2| 2x4 Pinheader. pitch terminal: 2.54mm |
|1 | R1, R2 | resistors for D1 and D2 |
|1 | R3 | 10KΩ PULL-UP resistor for DATA gamepad pin |
|1 | SW1,SW2 | Tact switches from Arduino breadboard projects |
|1 | U1 | DIP20 socket (width 15.24mm) + <a href="https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/hardware-overview-pro-micro">Arduino Pro Micro</a> |



