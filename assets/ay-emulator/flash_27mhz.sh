avrdude -p m328p -c USBasp -B4 -v -v -v -v -U flash:w:AY_Emul_247_2ch_m328_ay_speaker.hex -U eeprom:w:Conf_serial_27MHz_1_75Mhz.hex -U lfuse:w:0xee:m -U hfuse:w:0xdf:m -U efuse:w:0xfd:m
