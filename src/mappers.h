#ifndef __MAPPERS__
#define __MAPPERS__

#include "extern_structs.h"

void mapper_write(Cpu6502* cpu, uint16_t addr, uint8_t val);
void change_nt_mirroring(Cpu6502* cpu);
void init_mapper(Cartridge* cart, Cpu6502* cpu, Ppu2A03* ppu);

//void mapper_003(struct Cartridge* cart); /* CROM mapper */

#endif /* __MAPPERS__ */
