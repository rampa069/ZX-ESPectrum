#include "Arduino.h"
#include "PS2Kbd.h"
#include "SPIFFS.h"
#include "FS.h"
#include "Z80.h"

extern byte *bank0;
extern int start_im1_irq;
extern byte borderTemp;
extern void Z80_SetRegs (Z80_Regs *Regs);

typedef int32_t      dword;
typedef signed char    offset;


void load_speccy()
{
  File  lhandle;
  uint16_t  buf_p=0,quot=0,size_read,load_address;
  unsigned char csum,flag_byte,header_byte1;
  uint16_t cdd,f,ret,blocksize,data_leng,param1;
  uint16_t tap_leng,exec_address, buf_pale;
  char csum_ok[10];
  int file_id;
  byte *lbuffer;
  Z80_Regs i;

  Serial.printf("Free Heap before SNA: %d\n",system_get_free_heap_size());

  if(!SPIFFS.begin()){
        Serial.println("Internal memory Mount Failed");
        return;
    }

// open a file for input
//  lhandle = SPIFFS.open("/beep.sna", FILE_READ);
//    lhandle = SPIFFS.open("/fantasy.sna", FILE_READ);
//   lhandle = SPIFFS.open("/sppong.sna", FILE_READ);
//
//  lhandle = SPIFFS.open("/manic.sna", FILE_READ);
    lhandle = SPIFFS.open("/jetpac.sna", FILE_READ);
//  lhandle = SPIFFS.open("/jsw1.sna", FILE_READ);
//  lhandle = SPIFFS.open("/skooldz.sna", FILE_READ);

  size_read=0;
  if(lhandle!=NULL)
  {
    Serial.println("Loading:");
    //Read in the registers
    i.I=lhandle.read();
    i.HL2.B.l=lhandle.read();
    i.HL2.B.h=lhandle.read();
    i.DE2.B.l=lhandle.read();
    i.DE2.B.h=lhandle.read();
    i.BC2.B.l=lhandle.read();
    i.BC2.B.h=lhandle.read();
    i.AF2.B.l=lhandle.read();
    i.AF2.B.h=lhandle.read();
    i.HL.B.l=lhandle.read();
    i.HL.B.h=lhandle.read();
    i.DE.B.l=lhandle.read();
    i.DE.B.h=lhandle.read();
    i.BC.B.l=lhandle.read();
    i.BC.B.h=lhandle.read();
    i.IY.B.l=lhandle.read();
    i.IY.B.h=lhandle.read();
    i.IX.B.l=lhandle.read();
    i.IX.B.h=lhandle.read();

    byte inter =lhandle.read();
    i.IFF2 = ( inter & 0x04 ) ? 1 : 0;

    i.R =lhandle.read();
    i.AF.B.h=lhandle.read();
    i.AF.B.l=lhandle.read();
    i.SP.B.l=lhandle.read();
    i.SP.B.h=lhandle.read();
    i.IM = lhandle.read();
    byte bordercol =lhandle.read();

    Serial.printf("AF : %x\n",i.AF);
    Serial.printf("inter: %x\n",inter);
    i.SP.D=i.SP.B.l+i.SP.B.h * 0x100;
    Serial.printf("SP address: %x\n",i.SP.D);
    Serial.printf("R : %x\n",i.R);
    Serial.printf("IM : %x\n",i.IM);
    Serial.printf("Border : %x\n",bordercol);
    borderTemp = bordercol;


    i.IFF1 = i.IFF2;

    uint16_t thestack =  i.SP.D;
    uint16_t buf_p = 0;
    while (lhandle.available())
    {
      bank0[buf_p] = lhandle.read();
      buf_p++;
    }
    lhandle.close();
    Serial.printf("noof bytes: %x\n",buf_p);

    Serial.printf("STACK:\n");
    for(int yy = 0;yy < 16;yy++)
      Serial.printf("%x -> %x\n",thestack+yy,bank0[thestack - 0x4000 + yy]);

    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = bank0[offset]+0x100 * bank0[offset+1] ;
    Serial.printf("SP before: %x\n",i.SP.D);
    i.SP.D++;
    i.SP.D++;
    Serial.printf("SP after: %x\n",i.SP.D);

    i.PC.D=retaddr;
    //i.PC.D=0x8400;
    start_im1_irq=i.IM;
    Serial.printf("ret address: %x\n",retaddr);


    Z80_SetRegs (&i);
    Serial.printf("Free Heap after SNA: %d\n",system_get_free_heap_size());

  }
  else
  {
    Serial.println("Couldn't Open SNA file ");
    return;
  }
}
