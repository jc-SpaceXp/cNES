/*
 * Execute Functions (Opcode functions)
 *
 */

#ifndef __6502_FUNCTIONS__
#define __6502_FUNCTIONS__

#include "cpu.h"
#include <string.h>

/***************************
 * STORAGE                 *
 * *************************/

void execute_LDA(AddressMode mode, Cpu6502* CPU);
void execute_LDX(AddressMode mode, Cpu6502* CPU);
void execute_LDY(AddressMode mode, Cpu6502* CPU);
void execute_STA(Cpu6502* CPU);
void execute_STX(Cpu6502* CPU);
void execute_STY(Cpu6502* CPU);
void execute_TAX(Cpu6502* CPU);
void execute_TAY(Cpu6502* CPU);
void execute_TSX(Cpu6502* CPU);
void execute_TXA(Cpu6502* CPU);
void execute_TXS(Cpu6502* CPU);
void execute_TYA(Cpu6502* CPU);

/***************************
 * MATH                    *
 * *************************/

void execute_ADC(AddressMode mode, Cpu6502* CPU);
void execute_DEC(Cpu6502* CPU);
void execute_DEX(Cpu6502* CPU);
void execute_DEY(Cpu6502* CPU);
void execute_INC(Cpu6502* CPU);
void execute_INX(Cpu6502* CPU);
void execute_INY(Cpu6502* CPU);
void execute_SBC(AddressMode mode, Cpu6502* CPU);

/***************************
 * BITWISE                 *
 * *************************/

void execute_AND(AddressMode mode, Cpu6502* CPU);
void execute_ASL(AddressMode mode, Cpu6502* CPU);
void execute_BIT(Cpu6502* CPU);
void execute_EOR(AddressMode mode, Cpu6502* CPU);
void execute_LSR(AddressMode mode, Cpu6502* CPU);
void execute_ORA(AddressMode mode, Cpu6502* CPU);
void execute_ROL(AddressMode mode, Cpu6502* CPU);
void execute_ROR(AddressMode mode, Cpu6502* CPU);

/***************************
 * BRANCH                  *
 * *************************/

void execute_BCC(Cpu6502* CPU);
void execute_BCS(Cpu6502* CPU);
void execute_BEQ(Cpu6502* CPU);
void execute_BMI(Cpu6502* CPU);
void execute_BNE(Cpu6502* CPU);
void execute_BPL(Cpu6502* CPU);
void execute_BVC(Cpu6502* CPU);
void execute_BVS(Cpu6502* CPU);

/***************************
 * JUMP                    *
 * *************************/
void execute_JMP(Cpu6502* COU);
void execute_JSR(Cpu6502* CPU);
void execute_RTI(Cpu6502* CPU);
void execute_RTS(Cpu6502* CPU);

/***************************
 * Registers               *
 * *************************/

void execute_CLC(Cpu6502* CPU);
void execute_CLD(Cpu6502* CPU);
void execute_CLI(Cpu6502* CPU);
void execute_CLV(Cpu6502* CPU);
void execute_CMP(AddressMode mode, Cpu6502* CPU);
void execute_CPX(AddressMode mode, Cpu6502* CPU);
void execute_CPY(AddressMode mode, Cpu6502* CPU);
void execute_SEC(Cpu6502* CPU);
void execute_SED(Cpu6502* CPU);
void execute_SEI(Cpu6502* CPU);

/***************************
 * SYSTEM                  *
 * *************************/

void execute_BRK(Cpu6502* CPU);
void execute_NOP(void);
void execute_IRQ(Cpu6502* CPU);
void execute_NMI(Cpu6502* CPU); // Not an official opcode
void execute_DMA(void); // Not an official opcode

#endif /* __6502_FUNCTIONS__ */
