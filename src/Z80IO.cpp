/***                                                                      ***/
/***                                Z80IO.h                               ***/
/***                                                                      ***/
/*** This file contains the prototypes for the functions accessing memory ***/
/*** and I/O                                                              ***/
/***                                                                      ***/
/*** Copyright (C) Marcel de Kogel 1996,1997                              ***/
/***     You are not allowed to distribute this software commercially     ***/
/***     Please, notify me, if you make any changes to this file          ***/
/****************************************************************************/

/****************************************************************************/
/* Input a byte from given I/O port                                         */
/****************************************************************************/

#include "Arduino.h"
#include "Z80.h"
#include "paledefs.h"
#include "rom.h"
#include <stdint.h>

extern byte z80ports_in[32];
extern byte *bank0;
extern byte borderTemp;
extern byte soundTemp;
int start_im1_irq=0;
int start_ss_nmi=0;
int break_nmi=0;


void fastWrite(byte port,byte value)
{
  
  if (value > 0)
     REG_WRITE(GPIO_OUT_W1TS_REG, BIT5);
  else     
     REG_WRITE(GPIO_OUT_W1TC_REG, BIT5);
     
}

unsigned char testbit(char inbyt,int testbi)
{
  return((inbyt & (0x01<<testbi))>>testbi);
}

void bitset(unsigned char *vari,unsigned char posit,unsigned char valu)
{
        if (valu==0)
                *vari=(*vari & (255-((unsigned char)1 << posit)));
        else
                *vari=(*vari | ((unsigned char)1 << posit));
}

byte Z80_In (uint16_t Port)
{
  if((Port & 0x00ff)  == 0x00fe)
  {
    int16_t kbdportvalmasked = (Port & 0xff00) >> 8;
    int16_t kbdarrno = 0;
    switch(kbdportvalmasked)
    {
      case 0xfe: kbdarrno = 0;break; 
      case 0xfd: kbdarrno = 1;break; 
      case 0xfb: kbdarrno = 2;break; 
      case 0xf7: kbdarrno = 3;break; 
      case 0xef: kbdarrno = 4;break; 
      case 0xdf: kbdarrno = 5;break; 
      case 0xbf: kbdarrno = 6;break; 
      case 0x7f: kbdarrno = 7;break; 
    }
    return(z80ports_in[kbdarrno]);
  }
  return 0xff;
}

byte bank_latch = 0x00;
byte vid_latch = 0x00;


/****************************************************************************/
/* Output a byte to given I/O port                                          */
/****************************************************************************/
void Z80_Out (uint16_t Port,byte Value)
{

  Port = Port & 0xFF;
  switch (Port) 
  {
    case 0xfe:
    {     
      
      bitWrite(borderTemp,0,bitRead(Value,0));
      bitWrite(borderTemp,1,bitRead(Value,1));
      bitWrite(borderTemp,2,bitRead(Value,2));

      digitalWrite(SOUND_PIN,bitRead(Value,4));
      fastWrite(SOUND_PIN,testbit(Value,4));     
      break;
    }
  }
  
}


byte Z80_RDMEM(uint16_t A)
{
    if (A < 0x4000)
    {
        return specrom[A];
    }
    return bank0[A - 0x4000];
}

void Z80_WRMEM(uint16_t A,byte V)
{
      if(A >= 0x4000)
      {
         bank0[A - 0x4000]=V;
      }
}

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
//unsigned Z80_RDMEM(unsigned int A)
//{
//  
//  return bank0[A];
//}

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
//void Z80_WRMEM(unsigned int A,byte V)
//{
//  bank0[A] = V;
//}






void Z80_Patch (Z80_Regs *Regs)   /* Called when ED FE occurs. Can be used */
                                   /* to emulate disk access etc.           */
{

}
           /* This is called after IPeriod T-States */
                                   /* have been executed. It should return  */
                                   /* Z80_IGNORE_INT, Z80_NMI_INT or a byte */
                                   /* identifying the device (most often    */
                                   /* 0xFF)  */



int Z80_Interrupt(void)
{
//   return(Z80_IGNORE_INT );
//   return(0xff);
//return(0xff);
     

//  static int ii=0,jj=0;
  
  if ((start_ss_nmi==1))// || (break_nmi==1)) //not corrrect EMU
  {
    start_ss_nmi=0;
    return(Z80_NMI_INT);
  } 
  else if (get_IM()==1 && start_im1_irq==1 || (break_nmi==1))
  {
      start_im1_irq=0;
      return(0xff);//IM1 interrupt device  FF - rst 38
  }
  else
    return(Z80_IGNORE_INT );
}



void Z80_Reti (void)
{
}/* Called when RETI occurs               */

void Z80_Retn (void)
{
  
}/* Called when RETN occurs               */





/****************************************************************************/
/* Just to show you can actually use macros as well                         */
/****************************************************************************/
/*
 extern byte *ReadPage[256];
 extern byte *WritePage[256];
 #define Z80_RDMEM(a) ReadPage[(a)>>8][(a)&0xFF]
 #define Z80_WRMEM(a,v) WritePage[(a)>>8][(a)&0xFF]=v
*/

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define Z80_RDOP(A)    Z80_RDMEM(A)

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define Z80_RDOP_ARG(A)   Z80_RDOP(A)

/****************************************************************************/
/* Z80_RDSTACK() is identical to Z80_RDMEM() except it is used for reading  */
/* stack variables. In case of system with memory mapped I/O, this function */
/* can be used to slightly speed up emulation                               */
/****************************************************************************/
#define Z80_RDSTACK(A)    Z80_RDMEM(A)

/****************************************************************************/
/* Z80_WRSTACK() is identical to Z80_WRMEM() except it is used for writing  */
/* stack variables. In case of system with memory mapped I/O, this function */
/* can be used to slightly speed up emulation                               */
/****************************************************************************/
#define Z80_WRSTACK(A,V)  Z80_WRMEM(A,V)
