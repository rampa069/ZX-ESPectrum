#include <Arduino.h>
#include "paledefs.h"



#define BUFFER_SIZE 100
static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head, tail;


unsigned int shift = 0;
byte lastcode  = 0;
bool keyup = false;
bool shift_presed= false;
bool symbol_pressed = false;
byte rc = 0;
byte keymap[256];
byte oldKeymap[256];
extern bool debug_keyboard;


void kb_interruptHandler(void)
{
        static uint8_t bitcount=0;
        static uint8_t incoming=0;
        static uint32_t prev_ms=0;
        uint32_t now_ms;
        uint8_t n, val;
        
        
        int clock = digitalRead(KEYBOARD_CLK);
        if (clock== 1) 
           return;
           
        val = digitalRead(KEYBOARD_DATA);
        now_ms = millis();
        if (now_ms - prev_ms > 250) {
                bitcount = 0;
                incoming = 0;
        }
        prev_ms = now_ms;
        n = bitcount - 1;
        if (n <= 7) {
                incoming |= (val << n);
        }
        bitcount++;
        if (bitcount == 11) {
                
                if (1) {
                        if (keyup == true){
                          if(keymap[incoming] == 0) {
                            keymap[incoming]=1;
                          } else {
                            for(int gg = 0;gg < 256;gg++)
                              keymap[gg] = 1;
                          }
                            keyup=false;
                        }
                             else
                               keymap[incoming]=0;
                               
                        if (incoming==240)
                          keyup=true;
                           else keyup=false;
                          
                        Serial.printf("Incoming: %d\n",incoming);
                        

                        
                }
                bitcount = 0;
                incoming = 0;
        }
        //keymap[incoming]=0;
        //Serial.printf("Incoming: %d\n",incoming);
}


void kb_begin()
{
    pinMode(KEYBOARD_DATA,INPUT_PULLUP);
    pinMode(KEYBOARD_CLK,INPUT_PULLUP);
    digitalWrite(KEYBOARD_DATA,true);
    digitalWrite(KEYBOARD_CLK,true);
    attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
    for(int gg = 0;gg < 256;gg++) {
      keymap[gg] = 1;
      oldKeymap[gg] =1;
    }
    
}
