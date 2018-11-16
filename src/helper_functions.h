/*
 * Generic Functions 
 */

#ifndef __HELPER_FUNCTIONS__
#define __HELPER_FUNCTIONS__

#include "cpu.h"
#include "globals.h"
#include <stdio.h>

/*
 * ALL get_op functions have:
 * Parameters: 1. enum MODE address_mode = addressing mode
 *             2. *code = opcode
 *             3. CPU6502 *NESCPU = contains NES
 *             ... CPU registers etc.
 */
char instruction[14]; // Disassembler
char end[7]; // Disassembler
char append_int[6]; // Disassembler

/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op: fetches operand based on address modes */
void get_IMM_byte(CPU_6502* CPU);
void get_ZP_offset_address(uint8_t offset, CPU_6502* CPU);
void get_ABS_offset_address(uint8_t offset, CPU_6502* CPU);
void get_IND_address(CPU_6502* CPU);
void get_INDX_address(CPU_6502* CPU);
void get_INDY_address(CPU_6502* CPU);
unsigned page_cross_penalty(unsigned address_1, unsigned address_2);

/***************************
 * OTHER                   *
 * *************************/
void log_cpu_info(void); /* Return Status */
void update_cpu_info(void);

/***************************
 * ADD & SUB RELATED FUNCS *
 * *************************/

void Base10toBase2(uint8_t quotient, int *bin_array);
unsigned int Base2toBase10(int *bin_array, unsigned int dec_out);
void full_adder(int *bin_sum1, int *bin_sum2, int cIN, unsigned *cOUT, int *result);

/***************************
 * STACK                   *
 * *************************/

void stack_push(uint8_t value); /* Genric Push function */
uint8_t stack_pull(void); /* Genric Pop (Pull) function */

/***************************
 * FLAGS                   *
 * *************************/

/* Bits : 7 ----------> 0 */
/* Flags: N V - - D I Z C */

void update_FLAG_Z(uint8_t result);
void update_FLAG_N(uint8_t result);
void update_FLAG_V(int *bin_array1, int *bin_array2, int *result);
void update_FLAG_C(uint8_t cOUT);
void set_or_clear_CARRY(unsigned value);

#endif /* __HELPER_FUNCTIONS__ */
