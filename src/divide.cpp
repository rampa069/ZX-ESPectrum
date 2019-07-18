/*
 * Copyright (C) 2005-2009 Dusan Gallo
 * Email: dusky@hq.alert.sk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See the file COPYING.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * divIDE extension (http://baze.au.com/divide)
 *
 * I would like to thank to Zilog, baze, nairam for their useful
 * comments/informations. dusky
 *
 * $Rev: 12 $
 *
 */

#include "Arduino.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//#include "z80_type.h"
//#include "z80.h"
#include "Emulator/divIDE/demfir.h"
#include "Emulator/divIDE/divide.h"
//#include "interf.h"

#define DISK divide_drive[drive]
#define DISK2 divide_drive[drive^1]

#define STATUS_SET_ERR	0x01 // error
#define STATUS_SET_DRQ	0x08 // data request
#define STATUS_SET_DSC	0x10 // data seek complete
#define STATUS_SET_DF	0x20 // device fault
#define STATUS_SET_DRDY	0x40 // drive ready
#define STATUS_SET_BSY	0x80 // busy

#define STATUS_CLR_ERR	0xFE
#define STATUS_CLR_DRQ	0xF7
#define STATUS_CLR_DSC	0xEF
#define STATUS_CLR_DF	0xDF
#define STATUS_CLR_DRDY	0xBF
#define STATUS_CLR_BSY	0x7F

#define ERROR_SET_MED	0x01 // media error
#define ERROR_SET_NM	0x02 // no media
#define ERROR_SET_ABRT	0x04 // abort
#define ERROR_SET_MCR	0x08 // media change request
#define ERROR_SET_IDNF	0x10 // ID not founf
#define ERROR_SET_MC	0x20 // media changed
#define ERROR_SET_WP	0x40 // write protect
#define ERROR_SET_UNC	0x40 // uncorrectable data
#define ERROR_SET_ICRC	0x80 // interface crc error

#define ERROR_CLR_MED	0xFE
#define ERROR_CLR_NM	0xFD
#define ERROR_CLR_ABRT	0xFB
#define ERROR_CLR_MCR	0xF7
#define ERROR_CLR_IDNF	0xEF
#define ERROR_CLR_MC	0xDF
#define ERROR_CLR_WP	0xBF
#define ERROR_CLR_UNC	0xBF
#define ERROR_CLR_ICRC	0x7F


// DEMFIR image will be used if this variable is NULL
char *divide_image_path=NULL;
int divide_image_changed=0;

// DEMFIR operating system for divide (http://demfir.sourceforge.net)
extern const unsigned char demfir_imag[];

// mapped original ROM or divide memory?
int divide_memstate=0;
int divide_int_port=0;
int divide_int_state=0;

int divide_mb02_write_enable=0;

// flash write jumper
int divide_jp2=1;

// was memory page changed? (8k layout)
int divide_bank_changed=0;

// if should be done automapping
int divide_automap=0;
//CONMEM MAPRAM X X X X RAM_BANK1 RAM_BANK0
// forced memory mapping
#define DIVIDE_CONMEM(x) (x&0x80)
// EEPROM or RAM#3 mapped RO in 0K-8K range?
#define DIVIDE_MAPRAM(x) (x&0x40)
// which 8kbank is on 8K-16K ?
#define DIVIDE_BANK(x) (x&0x03)

//RETURN WRITEENABLE X X X X X BANK0
#define DIVIDE_RETURN(x) (x&0x80)
#define DIVIDE_WRITEENABLE(x) (x&0x40)
#define DIVIDE_MB02_BANK(x) (x&0x01)

#define DIVIDE_MAX_SEC_NR 96 // 96*512 = 48K .. in 48K mode spectrum this should be enough

// active drive (0='A:', 1='B:')
int divide_active_drive=0;

struct divide_part_struct {	// partition info
  int fd, len, ptr;
};

struct divide_drive_struct {
  char *path;		// image path
  int c,h,s;		// C/H/S geometry of disk
  int len;		// size of image (readed from identify file)
  int ptr;		// head ptr (not used)

  byte identify[512];	// identify virtual sector

// See README.divIDE for more information
  struct divide_part_struct mbr;		// Master Boot Record info
  struct divide_part_struct partition[4];	// partition info
//

  byte buffer[512*DIVIDE_MAX_SEC_NR];
  int buffer_ptr;
  int buffer_len;

  int cmd;		// pending command (read sector/write sector)
  int nsec;

// IDE registers (Read/Write)
  int port_AF;	// chs/lba
  int port_B3;	// chs/lba
  int port_B7;	// chs/lba
  int port_BB;	// chs/lba
  int port_A3;	// data
  int port_AB;	// sector cnt

// IDE registers (Write)
  int out_port_A7;	// features
  int out_port_BF;	// command

// IDE registers (Read)
  int in_port_A7;	// error
  int in_port_BF;	// status

};

struct divide_drive_struct divide_drive[2];

#define DIVIDE_READ_BUFF	1
#define DIVIDE_WRITE_BUFF	2

// due to not_so_good memory structure used in spectemu i had to switch
// memory by copying it, maybe i would rewrite it later
byte orig_rom[16384];
byte divide_eeprom[8192];
byte divide_ram[4][8192];

// C/H/S geometry -> LBA
int chs2lba(int drive, int c, int h, int s) {
  return (s+DISK.s*(h+c*DISK.h)-1);
}

// LBA geometry -> CHS doubleword
int lba2chs(int drive, int lba) {
  int c,h,s;

  lba++;
  s=lba%DISK.s;
  h=(lba/DISK.s)%DISK.h;
  c=(lba/DISK.s)/DISK.h;

  return (h<<24)|(c<<8)|s;
}

void divide_eeprom_save() {
  int i,fd;

  if (!divide_image_path || !divide_image_changed) return;
  if ((fd=open(divide_image_path, O_WRONLY))!=-1) {
    i=write(fd, divide_eeprom, 0x2000);
    close(fd);
    if (i!=0x2000) {
      Serial.printf("WARNING!!! EEPROM image is propably corrupted :( I cannot write 8KB.\n");
    }
  } else Serial.printf("No such file.\n");
  divide_image_changed=0;
}

void divide_eeprom_load() {
  int i,fd;

  if (!divide_image_path) return;
  if ((fd=open(divide_image_path, O_RDONLY))!=-1) {
    i=read(fd, divide_eeprom, 0x2000);
    close(fd);
    if (i!=0x2000) {
      Serial.printf("WARNING!!! EEPROM image is propably corrupted, check it first.\n");
      exit(0);
    }
  } else Serial.printf("No such file.\n");
  divide_image_changed=0;
}
// initialize variables
void divide_init() {
  int i, j, drive, part;

  // make copy of original spectrum memory (to recover it after unmaping divide memory)
  //memcpy(orig_rom, PRNM(proc).mem, 16384);

  // load EEPROM memory and clear RAM memory
  for(i = 0; i < 0x2000; i++)
      divide_eeprom[i] = demfir_imag[i];
  if (divide_image_path==NULL)
      Serial.printf("No EEPROM image defined. Using default DEMFIR image.\n");
  else {
      Serial.printf("Using file %s to load EEPROM image.\n", divide_image_path);
      divide_eeprom_load();
  }

  for(i = 0; i < 0x2000; i++)
    for (j=0;j<4;j++)
        divide_ram[j][i] = 0;

  // set default values

  for (drive=0; drive<2; drive++) {
    DISK.port_A3=0;
    DISK.port_AB=0;
    DISK.port_AF=0;
    DISK.port_B3=0;
    DISK.port_B7=0;
    DISK.port_BB=0;

    DISK.out_port_A7=0;
    DISK.out_port_BF=0;

    DISK.in_port_A7=0;
    DISK.in_port_BF=0;
    DISK.path=NULL;
    DISK.mbr.fd=-1;
    for (part=0; part<4; part++)
      DISK.partition[part].fd=-1;
  }
}

void divide_exit() {
  divide_detach_drive(1);
  divide_detach_drive(0);

  if (divide_image_path!=NULL) {
      Serial.printf("Using file %s to save EEPROM image.\n", divide_image_path);
      divide_eeprom_save();
  }
//  else  Serial.printf("No EEPROM image defined.\n");
}

// divide images are directories in fact
// it must contain files:
//   .identify (virtual sector containing drive firmware information)
//   .mbr (master boot record [and rest of track)
//   part1, part2, part3, part4 (partition image)
//   part5..partN, .ebr (other partitions, not implemented in this stage)
// this model was choosed to easy mount divide images under linux

void divide_attach_drive(int drive, char *path) {
  char *fname;
  int fd, r, len, part;
  byte identify[512];

  if (!path) return;

  // allocate buffer for filenames
  len=strlen(path) + 15;
  fname=(char *) malloc(len + 1);
  if (!fname) return;
  fname[len]='\0';

  // read contents of .identify "sector" if it doesn't exists ignore image
  snprintf(fname, len, "%s/.identify", path);
  if ((fd=open(fname, O_RDONLY))!=-1) {
    r=read(fd, identify, 512);
    close(fd);
    if (r!=512) return;

    // if another drive was mounted
    if ((DISK.path)) divide_detach_drive(drive);
    if (!(DISK.path=strdup(path))) return;

    memcpy(DISK.identify, identify, 512);

    // open Master Boot Record file
    snprintf(fname, len, "%s/.mbr", path);
    if ((DISK.mbr.fd=open(fname, O_RDWR))!=-1) {
      // length is in sectors (512bytes blocks), extra chunk is ignored (if size is not divisible by 512)
      DISK.mbr.len=(lseek(DISK.mbr.fd, 0, SEEK_END))>>9;
      DISK.mbr.ptr=0;
      // this is for realistic emulation of moving disk head (not used now)
      lseek(DISK.mbr.fd, DISK.mbr.ptr, SEEK_SET);
    } else {
      DISK.mbr.fd = -1;
      DISK.mbr.len = 0;
      DISK.mbr.ptr = 0;
    }

    // open partition files
    for (part=0; part<4; part++) {
      snprintf(fname, len, "%s/part%d", path, (part+1));
      if ((DISK.partition[part].fd=open(fname, O_RDWR))!=-1) {
        // length is in sectors (512bytes blocks), extra chunk is ignored (if size is not divisible by 512)
        DISK.partition[part].len=(lseek(DISK.partition[part].fd, 0, SEEK_END))>>9;
        DISK.partition[part].ptr=0;
        // this is for realistic emulation of moving disk head (not used now)
        lseek(DISK.partition[part].fd, DISK.partition[part].ptr, SEEK_SET);
      } else {
        DISK.partition[part].fd = -1;
        DISK.partition[part].len = 0;
        DISK.partition[part].ptr = 0;
      }
    }

    DISK.buffer_ptr=0;
    DISK.buffer_len=0;
    DISK.cmd=0;

    // C/H/S configuration from identify "sector"
    DISK.c=identify[108]+256*identify[109];
    DISK.h=identify[110]+256*identify[111];
    DISK.s=identify[112]+256*identify[113];

    if (!drive)
      Serial.println("Image successfully attached as drive 'A:'.");
    else
      Serial.println("Image successfully attached as drive 'B:'.");
  } else Serial.println("Image not found.");
  free(fname);
}

void divide_detach_drive(int drive) {

  int part;

  if (!DISK.path)
    return;

  free(DISK.path);
  DISK.path=NULL;

  if (DISK.mbr.fd!=-1) {
    close(DISK.mbr.fd);
    DISK.mbr.fd=-1;
    DISK.mbr.len=0;
    DISK.mbr.ptr=0;
  }
  for (part=0; part<4; part++)
    if (DISK.partition[part].fd!=-1) {
      close(DISK.partition[part].fd);
      DISK.partition[part].fd=-1;
      DISK.partition[part].len=0;
      DISK.partition[part].ptr=0;
    }
  if (!drive)
    Serial.println("Image successfully detached from drive 'A:'.");
  else
    Serial.println("Image successfully detached from drive 'B:'.");
}

void divide_select_drive(int drive) {
  char *name;

  if (!drive)
    Serial.println("Enter path to image (drive 'A:'):");
  else
    Serial.println("Enter path to image (drive 'B:'):");

  //name = spif_get_filename();
  strcpy(name,"drive.img");
  if(name == NULL) return;

  divide_detach_drive(drive);
  divide_attach_drive(drive, name);
}

// Well.. ready to go to the jungle? ;]

// IN wrapper
int divide_port_in(int portl) {
  int data, drive, lba, chs;

  drive = divide_active_drive;
  switch (portl) {
  case 0xA3: // data port
    if ( DISK.cmd == DIVIDE_READ_BUFF ) {
      if ( DISK.buffer_ptr < DISK.buffer_len ) {
        data = DISK.buffer[DISK.buffer_ptr];
        DISK.buffer_ptr++;
        if (!(DISK.buffer_ptr&0x1FF)) {
          DISK.port_AB--;
          if (DISK.buffer_ptr>0x200) {
            // LBA or CHS?
            if (DISK.port_BB & 0x40)
              lba = ((DISK.port_BB & 0x0F) << 24) | (DISK.port_B7 << 16) | (DISK.port_B3 << 8) | DISK.port_AF;
            else
              lba = chs2lba(drive, (DISK.port_B7 << 8) | DISK.port_B3, DISK.port_BB & 0x0F, DISK.port_AF);
            lba++;
            if (DISK.port_BB & 0x40) {
              DISK.port_BB&=0xF0;
              DISK.port_BB|=((lba>>24)&0x0F);
              DISK.port_B7=((lba>>16)&0xFF);
              DISK.port_B3=((lba>>8)&0xFF);
              DISK.port_AF=(lba&0xFF);
            } else {
              chs = lba2chs(drive, lba);
              DISK.port_BB&=0xF0;
              DISK.port_BB|=((chs>>24)&0x0F);
              DISK.port_B7=((chs>>16)&0xFF);
              DISK.port_B3=((chs>>8)&0xFF);
              DISK.port_AF=(chs&0xFF);
            }
          }
        }

        // readed whole buffer?
        if (DISK.buffer_ptr >= DISK.buffer_len)
          DISK.in_port_BF &= STATUS_CLR_DRQ;

        return data;
      } else {
        DISK.buffer_ptr = 0;
        DISK.buffer_len = 0;
        DISK.cmd = 0;
      }
    }
    return 0;
    break;
  case 0xA7: // error register
    return DISK.in_port_A7;
    break;
  case 0xAB: // sector count register
    return DISK.port_AB;
    break;
  case 0xBF: // status register
    return DISK.in_port_BF;
    break;
  case 0xBB: // drive/head register
    return DISK.port_BB;
    break;
  case 0xB7: // cylinder register high
    return DISK.port_B7;
    break;
  case 0xB3: // cylinder register low
    return DISK.port_B3;
    break;
  case 0xAF: // sector number register
    return DISK.port_AF;
    break;
  default: // unknown divide port?
//    Serial.printf("  unknown port #%02X\n", portl);
    ;
  }
  return -1;
}

// OUT wrapper
void divide_port_out(int portl, byte data) {
  int i, drive, part;
  int lba, tlba, chs;
  long size;

  drive = divide_active_drive;
  switch (portl) {
  case 0xE3: // divide_control_register
    // this port contains CONMEM, MAPRAM, x, x, x, x, RAM_BANK (2 bits)

    divide_bank_changed = (DIVIDE_BANK(divide_int_port) != DIVIDE_BANK(data));
    // MAPRAM cannot be cleared (1->0)
    divide_int_port = data | DIVIDE_MAPRAM(divide_int_port);
    divide_mb02_write_enable = (data & 0x40);
    break;
  case 0xBB: // drive/head register
    divide_active_drive = ((data >> 4) & 1);
    drive = divide_active_drive;
    DISK.port_BB = data;
    DISK2.port_BB = data;

    DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ & STATUS_CLR_ERR);
    DISK.in_port_BF |= (STATUS_SET_DRDY);
    break;
  case 0xB7: // cylinder register high
    DISK.port_B7 = data;
    DISK2.port_B7 = data;
    break;
  case 0xB3: // cylinder register low
    DISK.port_B3 = data;
    DISK2.port_B3 = data;
    break;
  case 0xAF: // sector number register
    DISK.port_AF = data;
    DISK2.port_AF = data;
    break;
  case 0xAB: // sector count register
    DISK.port_AB = data;
    DISK2.port_AB = data;
    break;
  case 0xBF: // command register
    switch (data) {
      case 0x00: // nop
        DISK.in_port_A7 |= ERROR_SET_ABRT;
        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ);
        DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_ERR);
        break;
      // silently return success result for these commands
      case 0x40: // read verify sectors with rerty
      case 0x41: // read verify sectors without rerty
      case 0xE0: // standby immediate
      case 0xE1: // idle immediate
      case 0xE2: // standby
      case 0xE3: // idle
      case 0xE5: // check power mode
      case 0xE6: // set sleep mode
      case 0xE7: // flush cache
        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ & STATUS_CLR_ERR);
        DISK.in_port_BF |= (STATUS_SET_DRDY);
        break;

      case 0x91: // initialise drive parameters
        if (!DISK.path || (DISK.port_AB == 0)) {
          // error
          DISK.in_port_A7 |= ERROR_SET_ABRT;
          DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DRQ);
          DISK.in_port_BF |= (STATUS_SET_DF | STATUS_SET_ERR);
          break;
        }
        // recalculate new geometry to fit CHS configuration
        size = DISK.identify[120] | (DISK.identify[121] << 8) | (DISK.identify[122] << 16) | (DISK.identify[123] << 24);
        if (size>16514064) size=16514064;

        DISK.h = (DISK.port_BB&15) + 1;
        DISK.s = DISK.port_AB;
        DISK.c = size / (DISK.h * DISK.s);
        if (DISK.c>65535) DISK.c=65535;

        size = DISK.c * DISK.h * DISK.s;

        DISK.identify[108] = DISK.c & 0xFF;
        DISK.identify[109] = DISK.c >> 8;

        DISK.identify[110] = DISK.h & 0xFF;
        DISK.identify[111] = DISK.h >> 8;

        DISK.identify[112] = DISK.s & 0xFF;
        DISK.identify[113] = DISK.s >> 8;

        DISK.identify[114] = size & 0xFF;
        DISK.identify[115] = (size >> 8) & 0xFF;
        DISK.identify[116] = (size >> 16) & 0xFF;
        DISK.identify[117] = (size >> 24);

        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ & STATUS_CLR_ERR);
        DISK.in_port_BF |= STATUS_SET_DRDY;
        break;

      case 0xA1: // identify drive (ATAPI)
      case 0xEC: // identify drive (ATA)
        if (!DISK.path) {
          DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DRDY & STATUS_CLR_DRQ);
          DISK.in_port_BF |= (STATUS_SET_DF | STATUS_SET_ERR);
          break;
        }
        memcpy(DISK.buffer, DISK.identify, 512);
        DISK.buffer_ptr = 0;
        DISK.buffer_len = 512;
        DISK.cmd=DIVIDE_READ_BUFF;

        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_ERR);
        DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_DRQ);
        break;

      case 0x20: // read sectors with retry
      case 0x21: // read sectors without retry
      case 0x22: // read long with retry
      case 0x23: // read long without retry
      case 0xC4: // read multiples
      case 0xC8: // read DMA with retry
      case 0xC9: // read DMA without retry

        if (!DISK.path || (DISK.port_AB>DIVIDE_MAX_SEC_NR)) {
          DISK.in_port_A7 &= (ERROR_CLR_MC & ERROR_CLR_MCR);
          DISK.in_port_A7 |= (ERROR_SET_UNC | ERROR_SET_IDNF | ERROR_SET_ABRT | ERROR_SET_NM);
          DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ);
          DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_ERR);
          break;
        }

        // LBA or CHS?
        if (DISK.port_BB & 0x40)
          lba = ((DISK.port_BB & 0x0F) << 24) | (DISK.port_B7 << 16) | (DISK.port_B3 << 8) | DISK.port_AF;
        else
          lba = chs2lba(drive, (DISK.port_B7 << 8) | DISK.port_B3, DISK.port_BB & 0x0F, DISK.port_AF);

        DISK.buffer_ptr = 0;
        DISK.buffer_len = 0;

        // send port_AB sectors
        for (i=0; i<((DISK.port_AB == 0) ? 256 : DISK.port_AB); i++) // 0 means 256 sectors
        {
          // read 1 sector only. we cannot read all sectors at once because we have disk splitted into blocks
          tlba=lba;
          if (tlba < DISK.mbr.len) {
            DISK.mbr.ptr = lseek(DISK.mbr.fd, (tlba << 9), SEEK_SET);
            read(DISK.mbr.fd, DISK.buffer + DISK.buffer_len, 512);
          } else {
            tlba-=DISK.mbr.len;
            for (part=0; part<4; part++) {
              if (tlba < DISK.partition[part].len) {
                DISK.partition[part].ptr = lseek(DISK.partition[part].fd, (tlba << 9), SEEK_SET);
                read(DISK.partition[part].fd, DISK.buffer + DISK.buffer_len, 512);
                break;
              }
              tlba-=DISK.partition[part].len;
            }
          }
          DISK.buffer_len += 512;
          lba++;
        }
        DISK.cmd = DIVIDE_READ_BUFF;
        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_ERR);
        DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_DRQ);
        break;
      case 0x30: // write sectors with retry
      case 0x31: // write sectors without retry
      case 0x32: // write long with retry
      case 0x33: // write long without retry
        if (!DISK.path || (DISK.port_AB>DIVIDE_MAX_SEC_NR)) {
          DISK.in_port_A7 &= (ERROR_CLR_WP & ERROR_CLR_MC & ERROR_CLR_MCR);
          DISK.in_port_A7 |= (ERROR_SET_IDNF | ERROR_SET_ABRT | ERROR_SET_NM);
          DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_DRQ);
          DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_ERR);
          break;
        }
        DISK.buffer_ptr = 0;
        DISK.buffer_len = 0;
        DISK.cmd = DIVIDE_WRITE_BUFF;
        DISK.nsec = -1;
        DISK.in_port_BF &= (STATUS_CLR_BSY & STATUS_CLR_DF & STATUS_CLR_ERR);
        DISK.in_port_BF |= (STATUS_SET_DRDY | STATUS_SET_DRQ);
        break;
      default:
//        Serial.printf("  unknown command #%02X\n", data);
        ;
    }
    break;
  case 0xA3: // data register
    if (!DISK.path || DISK.port_AB>DIVIDE_MAX_SEC_NR) {
      break;
    }
    if (DISK.cmd!=DIVIDE_WRITE_BUFF)
      break;
    // store a byte into buffer
    *(DISK.buffer + DISK.buffer_len)=data;
    DISK.buffer_len++;

    // if is stored whole sector write it to disk
    if (DISK.buffer_len == 512) {


      if (DISK.nsec>-1) lba=DISK.nsec;
      else {
        // LBA or CHS?
        if (DISK.port_BB & 0x40)
          lba = ((DISK.port_BB & 0x0F) << 24) | (DISK.port_B7 << 16) | (DISK.port_B3 << 8) | DISK.port_AF;
        else
          lba = chs2lba(drive, (DISK.port_B7 << 8) | DISK.port_B3, DISK.port_BB & 0x0F, DISK.port_AF);
      }
      // write n-th sector to disk
      tlba=lba;
      if (tlba < DISK.mbr.len) {
        DISK.mbr.ptr = lseek(DISK.mbr.fd, (tlba << 9), SEEK_SET);
        write(DISK.mbr.fd, DISK.buffer + DISK.buffer_ptr, 512);
      } else {
        tlba-=DISK.mbr.len;
        for (part=0; part<4; part++) {
          if (tlba < DISK.partition[part].len) {
            DISK.partition[part].ptr = lseek(DISK.partition[part].fd, (tlba << 9), SEEK_SET);
            write(DISK.partition[part].fd, DISK.buffer + DISK.buffer_ptr, 512);
            break;
          }
          tlba-=DISK.partition[part].len;
        }
      }
      DISK.buffer_len=0;
      DISK.port_AB--;
      if (DISK.port_BB & 0x40) {
        DISK.port_BB&=0xF0;
        DISK.port_BB|=((lba>>24)&0x0F);
        DISK.port_B7=((lba>>16)&0xFF);
        DISK.port_B3=((lba>>8)&0xFF);
        DISK.port_AF=(lba&0xFF);
      } else {
        chs = lba2chs(drive, lba);
        DISK.port_BB&=0xF0;
        DISK.port_BB|=((chs>>24)&0x0F);
        DISK.port_B7=((chs>>16)&0xFF);
        DISK.port_B3=((chs>>8)&0xFF);
        DISK.port_AF=(chs&0xFF);
      }
      lba++;
      DISK.nsec=lba;
    }
    // we are done with writing?
    if (!DISK.port_AB) {
      DISK.cmd=0;
      DISK.nsec=-1;
      DISK.in_port_BF &= STATUS_CLR_DRQ;
    }
    break;
  default:
//    Serial.printf("  unknown port #%02X\n", portl);
      ;
  }
}


void divide_premap(int addr) {
  // "fetch state"
  // mapper logic, see divIDE documentation for more info

  // check for divide memory connecting betadisk entry points
  if ((addr & 0xff00) == 0x3d00)
    divide_automap = 1;
  divide_mapper();
}

void divide_postmap(int addr) {
  // "M1 state"
  // mapper logic, see divIDE documentation for more info

  // check for divide memory connecting entry points
  if ((addr == 0x0000) ||
      (addr == 0x0008) ||
      (addr == 0x0038) ||
      (addr == 0x0066) ||
      (addr == 0x04C6) ||
      (addr == 0x0562)) divide_automap = 1;

  // check for original memory connecting entry points
  if ((addr & 0xfff8) == 0x1ff8) {
    divide_automap = 0;
  }
  divide_mapper();
}

void divide_mapper() {
  // if CONMEM is set, connect EEPROM (read only if JP2 is closed) and selected RAM bank (read/write)
  if (DIVIDE_CONMEM(divide_int_port)) { // map eeprom+ram
    if ((divide_memstate != 1) || divide_bank_changed) {
      //memcpy(PRNM(proc).mem, divide_eeprom, 0x2000);
      //memcpy(PRNM(proc).mem + 0x2000, divide_ram[DIVIDE_BANK(divide_int_port)], 0x2000);
    }
    divide_memstate = 1;
    divide_bank_changed = 0;
    return;
  }

  // if CONMEM isn't set and MAPRAM is set and:
  //   1) if we reached "connecting" entry point, connect RAM #3 bank (read only) and selected RAM bank (if it is 3 it is too read only, else it is writable)
  //   2) if we reached "disconnecting" entry point, connect original ZX Speccy ROM
  if (DIVIDE_MAPRAM(divide_int_port)) {

    // check for mb02 layout
    if (!divide_jp2 && (DIVIDE_BANK(divide_int_port) > 1 || divide_memstate == 3)) {

      if ((divide_memstate != 3) || divide_bank_changed) {
        //memcpy(PRNM(proc).mem, divide_ram[DIVIDE_MB02_BANK(divide_int_port)*2], 0x2000);
        //memcpy(PRNM(proc).mem + 0x2000, divide_ram[DIVIDE_MB02_BANK(divide_int_port)*2+1], 0x2000);
      }
      divide_memstate = 3;
      divide_bank_changed = 0;
      return;
    }
    if (divide_automap) { // map ram3+ram
      if ((divide_memstate != 2) || divide_bank_changed) {
        //memcpy(PRNM(proc).mem, divide_ram[3], 0x2000);
        //memcpy(PRNM(proc).mem + 0x2000, divide_ram[DIVIDE_BANK(divide_int_port)], 0x2000);
      }
      divide_memstate = 2;
      divide_bank_changed = 0;
    } else { // map original_rom
      if (divide_memstate) {
        //memcpy(PRNM(proc).mem, orig_rom, 0x4000);
      }
      divide_memstate = 0;
      divide_bank_changed = 0;
    }
    return;
  }

  // if JP2 is closed and:
  //   1) if we reached "connecting" entry point, connect EEPROM (read only) and selected RAM bank (read/write)
  //   2) if we reached "disconnecting" entry point, connect original ZX Speccy ROM
  if (divide_jp2) {
    if (divide_automap) { // map eeprom+ram
      if ((divide_memstate != 1) || divide_bank_changed) {
        //memcpy(PRNM(proc).mem, divide_eeprom, 0x2000);
        //memcpy(PRNM(proc).mem + 0x2000, divide_ram[DIVIDE_BANK(divide_int_port)], 0x2000);
      }
      divide_memstate = 1;
      divide_bank_changed = 0;
    } else { // map original_rom
      if (divide_memstate) {
        //memcpy(PRNM(proc).mem, orig_rom, 0x4000);
      }
      divide_memstate = 0;
      divide_bank_changed = 0;
    }
    return;
  }

  // else connect original ZX Speccy ROM
  if (divide_memstate) { // map original_rom
    //memcpy(PRNM(proc).mem, orig_rom, 0x4000);
  }
  divide_memstate = 0;
  divide_bank_changed = 0;
}

void divide_put_mem(int addr, byte *ptr, byte val) {
  switch (divide_memstate) {
    case 0:
      // original ZX Speccy ROM - no update memory
      break;
    case 1:
      // EEPROM + selected RAM bank
      if (addr<0x2000) {
        // is EEPROM writable?
        if (divide_jp2) break;
        *ptr=val;
        // we have to make this change in divide memory too (not really mapped, only copied)
        divide_eeprom[addr]=val;
        divide_image_changed=1;
        break;
      }
      *ptr=val;
      // we have to make this change in divide memory too (not really mapped, only copied)
      divide_ram[DIVIDE_BANK(divide_int_port)][addr-0x2000]=val;
      break;
    case 2:
      // RAM #3 bank + selected RAM bank
      // if it is 0k-8k page, write is disabled
      // if it is 8k-16k page, write is disabled if is selected RAM #3 bank
      if (addr<0x2000 || DIVIDE_BANK(divide_int_port)==3) break;
      *ptr=val;
      // we have to make this change in divide memory too (not really mapped, only copied)
      divide_ram[DIVIDE_BANK(divide_int_port)][addr-0x2000]=val;
      break;
    case 3:
      if (!divide_mb02_write_enable) break;
      *ptr=val;
      if (addr<0x2000)
        divide_ram[DIVIDE_MB02_BANK(divide_int_port)*2][addr]=val;
      else
        divide_ram[DIVIDE_MB02_BANK(divide_int_port)*2+1][addr-0x2000]=val;
      break;
    default:
//      Serial.printf("unknown divide state %d\n", divide_memstate);
      ;
  }
}

// JP2 (EEPROM "defender") switch
void divide_switch_jp2() {

  divide_jp2 ^= 1;
  Serial.printf( "JP2 %s", divide_jp2 ? "fused" : "opened");
}
