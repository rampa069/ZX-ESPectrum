#include "Arduino.h"
#include "PS2Kbd.h"
#include "SPIFFS.h"
#include "FS.h"
//#include "Z80.h"
//extern typedef Z80_Regs;

extern byte *bank0;
extern int start_im1_irq;
extern byte borderTemp;

typedef int32_t      dword;
typedef signed char    offset;

typedef union
{
#ifdef __128BIT__
 #ifdef LSB_FIRST
   struct { byte l,h,h2,h3,h4,h5,h6,h7,
                 h8,h9,h10,h11,h12,h13,h14,h15; } B;
   struct { int16_t l,h,h2,h3,h4,h5,h6,h7; } W;
   dword D;
 #else
   struct { byte h15,h14,h13,h12,h11,h10,h9,h8,
                 h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { int16_t h7,h6,h5,h4,h3,h2,h,l; } W;
   dword D;
 #endif
#elif __64BIT__
 #ifdef LSB_FIRST
   struct { byte l,h,h2,h3,h4,h5,h6,h7; } B;
   struct { int16_t l,h,h2,h3; } W;
   dword D;
 #else
   struct { byte h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { int16_t h3,h2,h,l; } W;
   dword D;
 #endif
#else
 #ifdef LSB_FIRST
   struct { byte l,h,h2,h3; } B;
   struct { int16_t l,h; } W;
   dword D;
 #else
   struct { byte h3,h2,h,l; } B;
   struct { int16_t h,l; } W;
   dword D;
 #endif
#endif
} pair;

typedef struct
{
  pair AF,BC,DE,HL,IX,IY,PC,SP;
  pair AF2,BC2,DE2,HL2;
  int16_t IFF1,IFF2,HALT,IM,I,R,R2;
} Z80_Regs;

extern void Z80_SetRegs (Z80_Regs *Regs);

void load_speccy()
{
  File  lhandle,lhandle2;
  uint16_t  buf_p=0,quot=0,size_read,load_address;
  unsigned char csum,flag_byte,header_byte1;
  uint16_t cdd,f,ret,blocksize,data_leng,param1;
  uint16_t tap_leng,exec_address, buf_pale;
  char csum_ok[10];
int tape_type = 0;
//  pump_string("mload\"\"\x0d");
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
    Serial.print("inter address: ");
    Serial.println((unsigned int)inter, HEX);

    i.IFF2 = ( inter & 0x04 ) ? 1 : 0;

    i.R =lhandle.read();
 Serial.print("R : ");
    Serial.println((uint8_t)i.R, HEX);

    i.AF.B.h=lhandle.read();
    i.AF.B.l=lhandle.read();
 Serial.print("AF : ");
    Serial.print((unsigned int)i.AF.B.l, HEX);
    Serial.println((unsigned int)i.AF.B.h, HEX);

    i.SP.B.l=lhandle.read();
    i.SP.B.h=lhandle.read();

    Serial.print("SP address: ");
    Serial.print((unsigned int)i.SP.B.l, HEX);
    Serial.println((unsigned int)i.SP.B.h, HEX);
    i.SP.D=i.SP.B.l+i.SP.B.h * 0x100;
    Serial.print("SP address (D): ");
    Serial.println((unsigned int)i.SP.D, HEX);

    i.IM = lhandle.read();
    Serial.print("IM : ");
    Serial.println((unsigned int)i.IM, HEX);

    byte bordercol =lhandle.read();
    Serial.print("Border : ");
    Serial.println((unsigned int)bordercol, HEX);
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
    Serial.print("noof bytes: ");
    Serial.println(buf_p);


    Serial.println("STACK:");
    for(int yy = 0;yy < 16;yy++)
          Serial.println(bank0[thestack - 0x4000 + yy], HEX);



    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = bank0[offset]+0x100 * bank0[offset+1] ;
    Serial.print("sp before");
    Serial.println((uint16_t) i.SP.D, HEX);
    i.SP.D++;
    i.SP.D++;
    Serial.print("sp after");
    Serial.println((uint16_t) i.SP.D, HEX);

    i.PC.D=retaddr; //dont work as expected. :-(
    //i.PC.D=0x8400;
    start_im1_irq=i.IM;
    Serial.print("retn address: ");
    Serial.println(retaddr, HEX);

    Serial.print ("Calculated PC: ");
    Serial.println(i.PC.D, HEX);


    Z80_SetRegs (&i);
    Serial.printf("Free Heap after SNA: %d\n",system_get_free_heap_size());

  }
  else
  {
    Serial.println("Couldn't Open SNA file ");
    return;
  }
}
