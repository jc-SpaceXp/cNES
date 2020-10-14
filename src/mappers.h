#ifndef __MAPPERS__
#define __MAPPERS__

#include "extern_structs.h"

struct Cartridge; // Forward decleratrion

void mapper_000(struct Cartridge* cart, Cpu6502* cpu, Ppu2A03* ppu); /* NROM mapper */
//void mapper_001(struct Cartridge* cart); /* MMC1 mapper */
//void mapper_003(struct Cartridge* cart); /* CROM mapper */

#endif /* __MAPPERS__ */
