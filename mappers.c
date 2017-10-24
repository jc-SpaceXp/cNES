#include "mappers.h"
#include "cart.h"
#include <stdio.h>

/* Mapper Registers & Banks */
uint8_t *PRG_ROM_BANK_1; /* till BFFF */
uint8_t *PRG_ROM_BANK_2; /* till FFFF */

/* NROM mapper */
void mapper_000(Cartridge* cart)
{
	uint8_t *PRG_ROM_BANK_1 = &NES->RAM[0x8000]; /* till BFFF */
	uint8_t *PRG_ROM_BANK_2 = &NES->RAM[0xC000]; /* till FFFF */
	if (cart->prg_rom.size == 16 * KiB) {
		memcpy(PRG_ROM_BANK_1, cart->prg_rom.data, 16 * KiB); /* First 16KiB */
		memcpy(PRG_ROM_BANK_2, PRG_ROM_BANK_1, 16 * KiB); /* Last 16KiB */
	} else {
		memcpy(PRG_ROM_BANK_1, cart->prg_rom.data, 32 * KiB);
	}
	free(cart->prg_rom.data);
}
