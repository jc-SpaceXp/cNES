/*
 * Generic Functions 
 */

#ifndef __HELPER_FUNCTIONS__
#define __HELPER_FUNCTIONS__

#include "cpu.h"
#include <stdio.h>

/*
 * ALL get_op functions have:
 * Parameters: 1. enum MODE address_mode = addressing mode
 *             2. *code = opcode
 *             3. CPU6502 *NESCPU = contains NES
 *             ... CPU registers etc.
 */
char instruction[18]; // Disassembler
char end[10]; // Disassembler
char append_int[6]; // Disassembler

/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op: fetches operand based on address modes */
void get_IMM_byte(Cpu6502* CPU);
void get_ZP_offset_address(uint8_t offset, Cpu6502* CPU);
void get_ABS_offset_address(uint8_t offset, Cpu6502* CPU);
void get_IND_address(Cpu6502* CPU);
void get_INDX_address(Cpu6502* CPU);
void get_INDY_address(Cpu6502* CPU);
unsigned page_cross_penalty(unsigned address_1, unsigned address_2);

/***************************
 * OTHER                   *
 * *************************/
void log_cpu_info(Cpu6502* CPU); /* Return Status */
void update_cpu_info(Cpu6502* CPU);

/***************************
 * STACK                   *
 * *************************/

void stack_push(Cpu6502* NES, uint8_t value); /* Genric Push function */
uint8_t stack_pull(Cpu6502* NES); /* Genric Pop (Pull) function */

/***************************
 * FLAGS                   *
 * *************************/

/* Bits : 7 ----------> 0 */
/* Flags: N V - - D I Z C */

void update_flag_z(Cpu6502* NES, uint8_t result);
void update_flag_n(Cpu6502* NES, uint8_t result);
void update_flag_v(Cpu6502* NES, bool overflow);
void update_flag_c(Cpu6502* NES, int carry_out);

#endif /* __HELPER_FUNCTIONS__ */
