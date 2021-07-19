#include "Emulator/Memory.h"
#include "Emulator/z80main.h"
#include "ZX-ESPectrum.h"

#include <FS.h>

#ifdef VGA32V14
#include <SD.h>
#else
#include <SPIFFS.h>
#endif

extern CONTEXT _zxContext;

struct z80snapshot {
	uint8_t       A, F, B, C, D, E, H, L, AA, FF, BB, CC, DD, EE, HH, LL, R, I, IFF1, IFF2, Imode;
	bool          issue_3;
	uint16_t      PC, IX, IY, SP;
	uint8_t       type;        // bit 0/1: 48K/128K/+3
	uint8_t       border;      // border color
	uint8_t       pager;       // content of pagination register in 128K mode
	unsigned int  found_pages; // bit=1: page exists. bit=0: page don't exists.
	unsigned char ay_regs[16];
	unsigned char ay_latch;
	unsigned char joystick;
};


void load_cpu(z80snapshot *snap){
	Serial.printf("AA:%d FF:%d BB:%d CC:%d DD:%d EE:%d HH:%d LL:%d\n", snap->AA, snap->FF,
	       snap->BB, snap->CC, snap->DD, snap->EE, snap->HH, snap->LL);
	_zxCpu.registers.byte[Z80_A] = snap->AA;
	_zxCpu.registers.byte[Z80_F] = snap->FF;
	_zxCpu.registers.byte[Z80_B] = snap->BB;
	_zxCpu.registers.byte[Z80_C] = snap->CC;
	_zxCpu.registers.byte[Z80_D] = snap->DD;
	_zxCpu.registers.byte[Z80_E] = snap->EE;
	_zxCpu.registers.byte[Z80_H] = snap->HH;
	_zxCpu.registers.byte[Z80_L] = snap->LL;

	_zxCpu.alternates[Z80_HL] = _zxCpu.registers.word[Z80_HL];
    _zxCpu.alternates[Z80_DE] = _zxCpu.registers.word[Z80_DE];
    _zxCpu.alternates[Z80_BC] = _zxCpu.registers.word[Z80_BC];
    _zxCpu.alternates[Z80_AF] = _zxCpu.registers.word[Z80_AF];

	Serial.printf("A:%d F:%d B:%d C:%d D:%d E:%d H:%d L:%d\n", snap->A, snap->F,
	       snap->B, snap->C, snap->D, snap->E, snap->H, snap->L);
	_zxCpu.registers.byte[Z80_A] = snap->A;
	_zxCpu.registers.byte[Z80_F] = snap->F;
	_zxCpu.registers.byte[Z80_B] = snap->B;
	_zxCpu.registers.byte[Z80_C] = snap->C;
	_zxCpu.registers.byte[Z80_D] = snap->D;
	_zxCpu.registers.byte[Z80_E] = snap->E;
	_zxCpu.registers.byte[Z80_H] = snap->H;
	_zxCpu.registers.byte[Z80_L] = snap->L;

	//R, I, IFF1, IFF2, Imode;
	Serial.printf("IX:%d IY:%d SP:%d PC:%d\n", snap->IX, snap->IY, snap->SP, snap->PC);

	_zxCpu.registers.word[Z80_IX] = snap->IX;
	_zxCpu.registers.word[Z80_IY] = snap->IY;
	_zxCpu.registers.word[Z80_SP] = snap->SP;
	_zxCpu.pc = snap->PC;
		
	Serial.printf("I:%d R:%d\n", snap->I, snap->R);
	_zxCpu.i = snap->I;
	_zxCpu.r = snap->R;
	
	Serial.printf("IFF1:%d IFF2:%d Imode:%d Border:%d\n", snap->IFF1, snap->IFF2,snap->Imode, snap->border);
	_zxCpu.iff1 = snap->IFF1;
	_zxCpu.iff2 = snap->IFF2;

	_zxCpu.im = snap->Imode;

	borderTemp = snap->border;

	Serial.printf("page port 0x7ffd: %u\n", snap->pager);
	_zxContext.output(0xfd,0x7f,snap->pager);

}

void load_uncompressed_z80(File *file, int length, uint16_t memStart){
	byte b;
	for(int i=0;i<length;i++){
		b = file->read();
		writebyte(memStart+i,b);
	}
}

void load_compressed_z80(File *file, int length, uint16_t memStart) {
	unsigned char byte_loaded, EDfound, counter;
	int           position;

	counter     = 0; //the x y uncompression counter
	byte_loaded = 0; //last byte loaded
	EDfound     = 0; //if two EDs are found
	position    = memStart;

	do {
		if (counter) {
			writebyte(position++,byte_loaded);
			counter--;
			continue;
		} else {
			byte_loaded= file->read();
		}

		if (EDfound == 2) { // we have two EDs
			counter = byte_loaded;
			byte_loaded= file->read();
			EDfound = 0;
			continue;
		}

		if (byte_loaded == 0xED) {
			EDfound++;
		} else {
			if (EDfound == 1) { // we found single ED xx. We write ED and xx
				writebyte(position++, 0xED);
				EDfound          = 0;
			}
			if (position >= length+memStart) {
				break;
			}
			writebyte(position++,byte_loaded);
		}
	} while (position < memStart+length);
}


//https://worldofspectrum.org/faq/reference/z80format.htm
int load_z80_file(File *file) {
	struct z80snapshot *snap;
	unsigned char       header1[30], header2[56], type, compressed, page, pageHeader[3];
	int   size = 0, length, bucle;

	if (file == NULL) {
		return -1; // error openning the file
	}

	snap = (struct z80snapshot *) malloc(sizeof(struct z80snapshot));

	size = file->size();
    
	Serial.println("Read Z80 file");

	for (int i = 0; i < 16; i++) {snap->ay_regs[i] = 0;}


	Serial.println("Read header (first 30 bytes)");
	file->read(header1,30);

	if ((header1[6] == 0) && (header1[7] == 0)) { // extended Z80
		Serial.println("It's an extended Z80 file");
		type = 1;                     // new type

		file->read(header2,2); // read the length of the extension header

		size = ((int) header2[0]) + ((int) header2[1] << 8);
		if (size > 54) {
			//file->close();
			Serial.println("Not suported Z80 file");
			free(snap);
			return -3; // not a supported Z80 file
		}
		Serial.printf("Header2 Length: %d\n", size);
		file->read(header2+2,size);//load the rest of header2

		if (size == 23) { // z80 ver 2.01
			switch (header2[4]) {
				case 0:
				case 1:
					snap->type = 0; // 48K
				break;

				case 3:
				case 4:
					snap->type = 1; // 128K
				break;

				default:
					Serial.println("Hardware Mode not suported Z80 file");
					free(snap);
					return -3; // not a supported Z80 file
				break;
			}
		} else {
			// z80 ver 3.0x
			switch (header2[4]) {
				case 0:
				case 1:
				case 3:
					snap->type = 0; // 48K
				break;

				case 4:
				case 5:
				case 6:
					snap->type = 1; // 128K
				break;

				default:
					free(snap);
					return -3; // not a supported Z80 file
				break;
			}
		}
	} else {
		Serial.println("Old type z80");
		type       = 0; // old type
		snap->type = 0; // 48k
	}

	if (header1[29] & 0x04) {
		Serial.println("Issue 2");
		snap->issue_3 = false; // issue2
	} else {
		Serial.println("Issue 3");
		snap->issue_3 = true; // issue3
	}

	snap->A = header1[0];
	snap->F = header1[1];
	snap->C = header1[2];
	snap->B = header1[3];
	snap->L = header1[4];
	snap->H = header1[5];
	if (type) {
		snap->PC = ((uint16_t) header2[2]) + ((uint16_t) header2[3] << 8);
		for (int i = 0; i < 16; i++) {
			snap->ay_regs[i] = header2[9 + i];
		}
		snap->ay_latch = header2[8];
	} else {
		snap->PC = ((uint16_t) header1[6]) + ((uint16_t) header1[7] << 8);
	}

	snap->SP = ((uint16_t) header1[8]) + ((uint16_t) header1[9] << 8);
	snap->I  = header1[10];
	snap->R  = (header1[11] & 0x7F);

	if (header1[12] == 255) {
		Serial.println("Header1 Byte 12 is 255! set this to 1\n");
		header1[12] = 1;
	}

	if (header1[12] & 0x01)
		{ snap->R |= 0x80; }

	snap->border = (header1[12] >> 1) & 0x07;

	compressed = ((header1[12] & 32) || (type)) ? 1:0;
	
	snap->E  = header1[13];
	snap->D  = header1[14];
	snap->CC = header1[15];
	snap->BB = header1[16];
	snap->EE = header1[17];
	snap->DD = header1[18];
	snap->LL = header1[19];
	snap->HH = header1[20];
	snap->AA = header1[21];
	snap->FF = header1[22];
	snap->IY = ((uint16_t) header1[23]) + 256 * ((uint16_t) header1[24]);
	snap->IX = ((uint16_t) header1[25]) + 256 * ((uint16_t) header1[26]);

	snap->IFF1 = (header1[27] == 0) ? 0:1;
	snap->IFF2 = (header1[28] == 0) ? 0:1; 
	
	snap->Imode = (header1[29] & 0x03);
	
	snap->joystick = ((header1[29] >> 6) & 0x03);

	if (type) 
		{ snap->pager = header2[5]; }

	if (type) { // extended z80
		if (snap->type == 1) { // 128K snapshot
			byte bank=bank_latch;
			while (file->available()){ 
				file->read(pageHeader,3);
				if (!file->available())
					break;
				
				length = ((int) pageHeader[0]) + ((int) pageHeader[1] << 8);
				page = pageHeader[2];  //reading the page index
				
				Serial.printf("Loading page %d of length %d\n", page, length);
				
				uint16_t memStart = 0xC000; //this is meant as a flag to skip loading this page
				bank_latch = page-3; //switching the memory page
				/*
				switch (page) {	
					case 4:
						memStart=0x8000;
					break;

					case 5:
						output(0x7F,0x00,3);
						memStart=0xC000;
					break;

					case 8:
						memStart=0x4000;
				break;

				}
				*/

				if (memStart<0xFFFF){
					if (length == 0xFFFF) { // uncompressed raw data
						Serial.printf("Load uncompressed Block at page %u memStart=%u\n ",page,memStart );
						load_uncompressed_z80(file,16384,memStart);
					} else {
						Serial.printf("Load compressed Block at page %u memStart=%u\n",page,memStart);
						load_compressed_z80(file, 16384, memStart);
					}
				}
				else
					file->seek(file->position()+length); //skip this block
			}
			bank_latch=bank; //switch back to the saved bank
		} else {  //48K
			while (file->available()) {
				file->read(pageHeader,3);
				if (!file->available()) {
					break;
				}
				length = ((int) pageHeader[0]) + ((int) pageHeader[1] << 8);
				page = pageHeader[2];

				Serial.printf("Page header: page=%u length=%u\n",page,length);

				uint16_t memStart = 0xFFFF; //this is meant as a flag to skip loading this page
				switch (page) {
					case 4:
						memStart = 0x8000;
					break;

					case 5:
						memStart = 0xC000;
					break;

					case 8:
						memStart = 0x4000;
					break;

					default:
						//page = 11;
					break;
				}
				if (memStart!=0xFFFF){
					if (length == 0xFFFF) { // uncompressed raw data
						Serial.printf("Load uncompressed Block at page %u\n",page );
						load_uncompressed_z80(file,16384,memStart);
					} else {
						Serial.printf("Load compressed Block at page %u\n",page );
						load_compressed_z80(file, 16384,memStart);
					}
				}
				else
					file->seek(file->position()+length); //skip this block	
			}
		}
	} else {
		if (compressed) {
			Serial.println("48k compressed z80 loader");

			//single 48K block compressed
			load_compressed_z80(file, 49152, 0x4000);
		} else {
			Serial.println("48k uncompressed z80 loader");
			Serial.println("Load uncompressed blocks");
			load_uncompressed_z80(file,16384,0x4000);
			load_uncompressed_z80(file,16384,0x8000);
			load_uncompressed_z80(file,16384,0xC000);
		}
	}

	load_cpu(snap);
	free(snap);
	return 0; // all right
}
