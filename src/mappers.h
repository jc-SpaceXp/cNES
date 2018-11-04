#ifndef __MAPPERS__
#define __MAPPERS__

#include "cpu.h"
#include "ppu.h"

struct Cartridge; /* can't include cart.h as breaks compiler include in .c file */

void mapper_000(struct Cartridge* cart); /* NROM mapper */
void mapper_001(struct Cartridge* cart); /* MMC1 mapper */
void mapper_003(struct Cartridge* cart); /* CROM mapper */

#endif /* __MAPPERS__ */
