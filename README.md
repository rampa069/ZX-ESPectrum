# We've Moved! Our Emulator has a New Home

Greetings fellow retro gaming enthusiasts!

We have some exciting news to share - our beloved emulator has packed its bags and moved to a new home at [EremusOne/ZX-ESPectrum-IDF](https://github.com/EremusOne/ZX-ESPectrum-IDF). (**IDF** - Incredible Digital Fun!)

We know it's a little sad to say goodbye to the old repository, but trust us, the new one is so much better. It's like upgrading from a rubber-keyed Spectrum to a 128k, but without the disappointment of finding out you still can't load anything on tape.

We've added some great new features and improved performance, so you can spend less time waiting for games to load and more time actually playing them. And, of course, we've kept all the classic features that you know and love.

So, what are you waiting for? Come join us in our new home at EremusOne/ZX-ESPectrum-IDF, and experience the joy of retro gaming like never before.

Thanks for all the good times, rampa069/ZX-ESPectrum repository. You were a faithful companion during many late-night gaming sessions, but it's time to move on to bigger and better things.

See you at the new repository!

Yours in retro gaming,

The Dev Team

---

# ZX-ESPectrum (old readme for the history)

An emulation of the ZX-Spectrum computer on an ESP32 chip with VGA output based on bitluni's driver.

## Features

- VGA output, 8 or 16 bits.
- Beeper digital output.
- Accurate Z80 emulation.
- Spectrum 16/48 achitecture emulation without PSRAM.
- Spectrum 128/+2/+3 architecture emulation with PSRAM.
- PS/2 Keyboard.
- VGA OSD menu: Configuration, and architecture selection, ROM and SNA.
- Tape save and loading.
- SNA snapshot loading.
- Internal SPIFFS support.

## Work in progress

- AY-3-8910 emulation and sound output with dedicated chip.
- SD card support.
- DivIDE emulation.
- Dedicated motherboard design.
- Joystick support.
- USB keyboard.
- OTA: Over the Air updates.

## Compiling and installing

GNU/Linux, MacOS/X and Windows supported.

#### Install platformIO:

- They have an extension for Atom and VSCode, and this is [the webpage](https://platformio.org/).
- Select you board.
- Install Bitluni's ESP32Lib (use version 0.2.1, newer versions such as 0.3.3 will lead to compile errors)

#### Softlink and customize platformio.ini

```bash
ln -s platformio.ini.linux platformio.ini
# or in osx
ln -s platformio.ini.osx platformio.ini
```

#### Copy boot.cfg

```bash
cp data/boot.cfg.orig boot.cfg
```

#### Upload the data filesystem

`PlatformIO > Project Tasks > Upload File System Image`

#### Compile and flash it

Right arrow at the bottom icon bar or `PlatformIO > Project Tasks > Build` and `PlatformIO > Project Tasks > Upload`.

## Hardware configuration and pinout

See pin assignment in include/def/hardware.h or change it to your own preference.

## Thanks to

- Idea from the work of Charles Peter Debenham Todd: [PaseVGA](https://github.com/retrogubbins/paseVGA).
- VGA Driver from [ESP32Lib by BitLuni](https://github.com/bitluni/ESP32Lib).
- PS/2 keyboard support based on [ps2kbdlib](https://github.com/michalhol/ps2kbdlib).
- Z80 Emulation derived from [z80emu](https://github.com/anotherlin/z80emu) authored by Lin Ke-Fong.
- DivIDE ideas (work in progress) taken from the work of Dusan Gallo.
- AY sound hardware emulation from [AVR-AY](https://www.avray.ru/).
- [Amstrad PLC](http://www.amstrad.com) for the ZX-Spectrum ROM binaries [liberated for emulation pourposes](http://www.worldofspectrum.org/permits/amstrad-roms.txt).
- [Nine Tiles Networs Ltd](http://www.worldofspectrum.org/sinclairbasic/index.html) for Sinclair BASIC.
- Gary Lancaster for the [+3e ROM](http://www.worldofspectrum.org/zxplus3e/).
- [Retroleum](http://blog.retroleum.co.uk/electronics-articles/a-diagnostic-rom-image-for-the-zx-spectrum/) for the diagnostics ROM.

## And all the involved people from the golden age

- [Sir Clive Sinclair](https://en.wikipedia.org/wiki/Clive_Sinclair).
- [Christopher Curry](https://en.wikipedia.org/wiki/Christopher_Curry).
- [The Sinclair Team](https://en.wikipedia.org/wiki/Sinclair_Research).
- [Lord Alan Michael Sugar](https://en.wikipedia.org/wiki/Alan_Sugar).
- [Investrónica team](https://es.wikipedia.org/wiki/Investr%C3%B3nica).
- [Sovietic cloners](https://en.wikipedia.org/wiki/List_of_ZX_Spectrum_clones).
- Queru's uncle Roberto for introducing him into the microcomputing world with a [Commodore VIC-20](https://en.wikipedia.org/wiki/Commodore_VIC-20).
- Queru's uncle Manolito for introducing him into the ZX-Spectrum world.
- Rampa's mother for the [Oric 1](https://en.wikipedia.org/wiki/Oric#Oric-1) and for inculcate her passion for electronics.

## And all the writters, hobbist and documenters

- [Microhobby magazine](https://es.wikipedia.org/wiki/MicroHobby).
- Dr. Ian Logan & Dr. Frank O'Hara for [The Complete Spectrum ROM Disassembly book](http://freestuff.grok.co.uk/rom-dis/).
- Chris Smith for the The [ZX-Spectrum ULA book](http://www.zxdesign.info/book/).
- Users from [Abadiaretro](https://abadiaretro.com/) and its Telegram group.
- Users from [El Mundo del Spectrum](http://www.elmundodelspectrum.com/) and its Telegram group.
- Users from Hardware Devs group.
- [The World of Spectrum](http://www.worldofspectrum.org/).

## A lot of programmers, especially

- [GreenWebSevilla](https://www.instagram.com/greenwebsevilla/) for its Fantasy Zone game and others.
- Julián Urbano Muñoz for [Speccy Pong](https://www.instagram.com/greenwebsevilla/).
- Others who have donated distribution rights for this project.
