/* Reads NES ROM and print's what instructions we encounter */
/* in Numerical order */
#ifndef __6502_disassembler
#define __6502_disassembler

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int Disassemble6502(unsigned char *code, int pc);

#endif /* __6502_disassembler */
