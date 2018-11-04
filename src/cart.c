#include "cart.h"
#include <stdlib.h>
#include <stdio.h>


/* iNES format */
int load_cart(Cartridge* cart, const char *filename, PPU_Struct *p)
{
	uint8_t header[16];
	int trainer;
	uint8_t mapper;
	long file_size;

	/* opening file and file testing */
	FILE *rom = fopen(filename, "rb");

	/* Error Detection */
	if (rom == NULL) {
		fprintf(stderr, "Error: couldn't open file.\n");
		exit (8);
	}
	
	/* Getting filesize */
	fseek(rom, 0L, SEEK_END);
	file_size = ftell(rom);
	rewind(rom);

	if (file_size < 0x4010) {
		fprintf(stderr, "Error: input file is too small.\n");
		fclose(rom);
		exit (8);
	}

	/* loading first 16 bytes of .nes file into header */
	if (fread(header, 1, 16, rom) != 16) {
		fprintf(stderr, "Error: unable to read ROM header.\n");
		fclose(rom);
		exit (8);
	}
	
	if (memcmp(header, "NES\x1A", 4) != 0) {
		fprintf(stderr, "Error: invalid nes file.\n");
		fclose(rom);
		exit (8);
	}

	/* parsing header */
	cart->prg_rom.size = 16 * (KiB) * header[4];
	cart->prg_ram.size = 8 * (KiB) * header[8];
	cart->chr_rom.size = 8 * (KiB) * header[5]; // Pattern table data (if any)
	mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);

	/* Flags 6 */
	if (!(header[6] & 0x08)) {
		if (header[6] & 0x01) {
			p->mirroring = 1;
		} else {
			p->mirroring = 0;
		}
	} else {
		p->mirroring = 4;
	}

	if ((header[6] & 0x04) == 0x04) {
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
	cart->prg_rom.data = (uint8_t*) malloc(cart->prg_rom.size);
	fread(cart->prg_rom.data, 1, cart->prg_rom.size, rom);

	/* loading data into chr_rom */
	cart->chr_rom.data = (uint8_t*) malloc(cart->chr_rom.size);
	fread(cart->chr_rom.data, 1, cart->chr_rom.size, rom);

	fclose(rom);

	/* Switch-case for mapper number */
	switch(mapper) {
	case 0:
		mapper_000(cart);
		break;
	default:
		printf("mapper not implemented yet\n");
		break;
	}

	return 0;
}
