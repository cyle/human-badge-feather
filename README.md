# Human Badge v0

The "HUMAN BADGE" is my attempt at a Defcon/tech electronic badge that does some neat things, especially when it's around other badges.

This is v0 because I'm still prototyping everything.

The overall idea is to make a badge (kind of like [the amazing Defcon 27 badge](http://www.grandideastudio.com/defcon-27-badge/)) that interacts with other badges. A kind of hackable Nintendo StreetPass. Your badge is either sending or listening for pulses, and those pulses can contain a little bit of info about you, or whatever you want. Receivers can see these things on their badge as they get them and look em up later.

The basic features include right now:

- An Adafruit Feather M0 w/ RFM95 900mhz radio and battery port and charging, as the heart.
- A little piezo speaker that beeps when a pulse is sent and plays a series of tones when one is received.
- A 1.3" square 240x240 display to tell you what it's doing, with a microSD card for storage.
- Some switches and a knob for controls.

Future plans:

- The screen should display your status and icon and who you've recently been pinged by.
- A serial terminal connection to the badge itself so you can set up what you want to send, and download what you've got.
- A proper custom PCB for everything!

## Code Requirements

- Arduino IDE 1.8.10
- [RadioHead library](http://www.airspayce.com/mikem/arduino/RadioHead/)
- Adafruit GFX, Zero DMA, ST7735 and ST7789, SPIFlash, and SdFat - Adafruit Fork libraries, via Arduino IDE's package manager.

Debugging via serial is essential. It'll tell you what you wanna know at 115200 baud.

## Physical Setup

If you want to breadboard it:

- @todo gotta redo these instructions now that it's a Feather M0 and not Arduino Nano

You should be able to hook up your Feather to your computer via USB and upload the sketch.

You'll need at least two setups to get the actual functionality of the thing, of course, so there's someone else for your badge to talk to.

## Resources

- [Adafruit GFX library reference and guide](https://learn.adafruit.com/adafruit-gfx-graphics-library)
- [Adafruit 1.3" display guide](https://learn.adafruit.com/adafruit-1-3-and-1-54-240-x-240-wide-angle-tft-lcd-displays)
