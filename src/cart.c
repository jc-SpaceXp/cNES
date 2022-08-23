#include "cart.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Cartridge* cart_init(void)
{
	Cartridge* cart = malloc(sizeof(Cartridge));
	if (!cart) {
		fprintf(stderr, "Failed to allocate memory for Cartridge\n");
	}

	return cart;
}

/* iNES format */
int load_cart(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu)
{
	uint8_t header[16];
	unsigned trainer;
	uint8_t mapper;
	long file_size;

	FILE* rom = fopen(filename, "rb");

	/* Error Detection */
	if (rom == NULL) {
		fprintf(stderr, "Error: couldn't open file.\n");
		return 8;
	}
	
	/* Getting filesize */
	fseek(rom, 0L, SEEK_END);
	file_size = ftell(rom);
	rewind(rom);

	if (file_size < 0x4010) {
		fprintf(stderr, "Error: input file is too small.\n");
		fclose(rom);
		return 8;
	}

	/* loading first 16 bytes of .nes file into header */
	if (fread(header, 1, 16, rom) != 16) {
		fprintf(stderr, "Error: unable to read ROM header.\n");
		fclose(rom);
		return 8;
	}
	
	if (memcmp(header, "NES\x1A", 4)) {
		fprintf(stderr, "Error: invalid nes file.\n");
		fclose(rom);
		return 8;
	}

	/* parsing header */
	cart->prg_rom.size = 16 * (KiB) * header[4];
	cart->prg_ram.size = 8 * (KiB) * header[8];
	cart->chr.rom_size = 8 * (KiB) * header[5]; // Pattern table data (if any)
	cart->chr.ram_size = 8 * (KiB) * !header[5];
	mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);
	cpu->cpu_mapper_io->mapper_number = mapper;

	/* Flags 6 */
	if (!(header[6] & 0x08)) {
		if (header[6] & 0x01) {
			ppu->mirroring = 1;
		} else {
			ppu->mirroring = 0;
		}
	} else {
		ppu->mirroring = 4;
	}

	if (header[6] & 0x04) {
		trainer = 512;
	} else {
		trainer = 0;
	}
		

	/* Flags 7 */

	/* Flags 9 */
	if (header[9] & 0x01) {
		cart->video_mode = PAL;
	} else {
		cart->video_mode = NTSC;
	}

	/* Flags 10 - not implimented - rarely used */

	/* Loading data into PRG_ROM */
	fseek(rom, trainer, SEEK_CUR); /* fseek has gone past header now needs to skip trainer */
	cart->prg_rom.data = malloc(cart->prg_rom.size);
	if (!cart->prg_rom.data) {
		fclose(rom);
		return 8;
	}
	fread(cart->prg_rom.data, 1, cart->prg_rom.size, rom);

	/* loading data into chr_rom */
	unsigned chr_size = cart->chr.rom_size ? cart->chr.rom_size : cart->chr.ram_size;
	if (chr_size) {
		cart->chr.data = malloc(chr_size);
		if (!cart->chr.data) {
			free(cart->prg_rom.data);
			fclose(rom);
			return 8;
		}
		fread(cart->chr.data, 1, chr_size, rom);
	}

	fclose(rom);

	/* Mapper select */
	init_mapper(cart, cpu, ppu);

	return 0;
}
