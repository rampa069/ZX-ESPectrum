/* z80user.h
 * Add your code here to interface the emulated system with z80emu. See towards
 * the end of the file for an example for running zextest.
 *
 * Copyright (c) 2016, 2017 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#ifndef __Z80USER_INCLUDED__
#define __Z80USER_INCLUDED__

#include <stdint.h>
#include "z80emu/z80emu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Write the following macros for memory access and input/output on the Z80. 
 *
 * Z80_FETCH_BYTE() and Z80_FETCH_WORD() are used by the emulator to read the
 * code (opcode, constants, displacement, etc). The upper 16-bit of the address
 * parameters is undefined and must be reset to zero before actually reading 
 * memory (use & 0xffff). The value x read, must be an unsigned 8-bit or 16-bit
 * value in the endianness of the host processor. 
 *
 * Z80_READ_BYTE(), Z80_WRITE_BYTE(), Z80_READ_WORD(), and Z80_WRITE_WORD()
 * are used for general memory access. They obey the same rules as the code 
 * reading macros. The upper bits of the value x to write may be non-zero.
 * Z80_READ_WORD_INTERRUPT() and Z80_WRITE_WORD_INTERRUPT() are same as 
 * respectively Z80_READ_WORD() and Z80_WRITE_WORD(), except they are only used
 * for interrupt generation.
 * 
 * Z80_INPUT_BYTE() and Z80_OUTPUT_BYTE() are for input and output. The upper
 * bits of the port number to read or write are always zero. The input byte x 
 * must be an unsigned 8-bit value. The value x to write is an unsigned 8-bit 
 * with its upper bits zeroed.
 *
 * All macros have access to the following three variables:
 *
 *      state           Pointer to the current Z80_STATE. Because the 
 *			instruction is currently executing, its members may not
 *			be fully up to date, depending on when the macro is 
 *			called in the process. It is rather suggested to access 
 *			the state only when the emulator is stopped. 
 *
 *      elapsed_cycles  Number of cycles emulated. If needed, you may add wait 
 *			states to it for slow memory accesses. Because the 
 *			macros are called during the execution of the current 
 *			instruction, this number is only precise up to the 
 *			previous one.
 *
 *	    context         This is the (void *) context passed to the emulation 
 *			functions.
 *
 * Except for Z80_READ_WORD_INTERRUPT and Z80_WRITE_WORD_INTERRUPT, all macros 
 * also have access to: 
 *
 *      number_cycles   Number of cycles to emulate. After executing each
 *			instruction, the emulator checks if elapsed_cycles is
 *			greater or equal to number_cycles, and will stops if 
 *			so. Hence you may decrease or increase the value of 
 *			number_cycles to stop the emulation earlier or later.
 * 			In particular, if you set it to zero, the emulator will
 * 			stop after completion of the current instruction. 
 *
 *      registers       Current register decoding table, use it to determine if
 * 			the current instruction is prefixed. It points on:
 *                      
 *				state->dd_register_table for 0xdd prefixes; 
 *                      	state->fd_register_table for 0xfd prefixes;
 *				state->register_table otherwise.
 *
 *      pc              Current PC register (upper bits are undefined), points
 *                      on the opcode, the displacement or constant to read for
 *                      Z80_FETCH_BYTE() and Z80_FETCH_WORD(), or on the next
 *                      instruction otherwise.
 *
 * Except for Z80_FETCH_BYTE(), Z80_FETCH_WORD(), Z80_READ_WORD_INTERRUPT, and 
 * Z80_WRITE_WORD_INTERRUPT, all other macros can know which instruction is 
 * currently executing:
 *
 *      opcode          Opcode of the currently executing instruction.
 *
 *      instruction     Type of the currently executing instruction, see
 *                      instructions.h for a list.
 */

typedef struct CONTEXT {
	uint8_t(*readbyte)(uint16_t);
	uint16_t(*readword)(uint16_t);
	void(*writebyte)(uint16_t, uint8_t);
	void(*writeword)(uint16_t, uint16_t);
	uint8_t(*input)(uint8_t, uint8_t);
	void(*output)(uint8_t, uint8_t, uint8_t);
} CONTEXT;

#define Z80_READ_BYTE(address, x)                          \
{                                                          \
        (x) = ((CONTEXT*)context)->readbyte(address);      \
}

#define Z80_WRITE_BYTE(address, x)                         \
{                                                          \
        ((CONTEXT*)context)->writebyte(address, x);        \
}

#define Z80_READ_WORD(address, x)                          \
{                                                          \
        (x) = ((CONTEXT*)context)->readword(address);      \
}

#define Z80_WRITE_WORD(address, x)                         \
{                                                          \
        ((CONTEXT*)context)->writeword(address, x);        \
}

#define Z80_INPUT_BYTE(portLow, portHigh, x)               \
{                                                          \
        (x) = ((CONTEXT*)context)->input(portLow, portHigh); \
}

#define Z80_OUTPUT_BYTE(portLow, portHigh, x)              \
{                                                          \
        ((CONTEXT*)context)->output(portLow, portHigh, x); \
}                                                                      

#define Z80_FETCH_BYTE(address, x)		Z80_READ_BYTE((address), (x))

#define Z80_FETCH_WORD(address, x)		Z80_READ_WORD((address), (x))

#define Z80_READ_WORD_INTERRUPT(address, x)	 Z80_READ_WORD((address), (x))

#define Z80_WRITE_WORD_INTERRUPT(address, x) Z80_WRITE_WORD((address), (x))

#ifdef __cplusplus
}
#endif

#endif
