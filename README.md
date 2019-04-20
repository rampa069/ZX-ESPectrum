# ZX-ESPectrum

An emulation of the ZX Spectrum computer on an ESP32 and VGA Screen using the
bitluni VGA card. based on the works of Charles Peter Debenham Todd

* original here: https://github.com/retrogubbins/paseVGA
* VGA Driver is from ESP32Lib by BitLuni https://github.com/bitluni/ESP32Lib
* You need to upload the SPIFFS filesystem using the plugin in arduino. https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
* Z80 core is by Marcel de Kogel
* Will load SNA snapshots from SPIFF space.
* set run_snaphot to false  to boot sinclair BASIC or set it to true and  change the SNA file  in Tapes.INO
* There is no copyrighted games in the data folder, if you have the right to use on, copy it to the data folder.
* Look/change paledefs.h to change vga, keyboard and buzzer pins
