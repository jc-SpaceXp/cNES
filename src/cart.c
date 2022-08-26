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

	cart->header = HEADERLESS;

	return cart;
}

static void log_cart_info(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu, uint8_t* header_bytes);

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

	if (!memcmp(header, "NES\x1A", 4)) {
		// bytes 10-15 should be filled w/ 0's however people often write in this space
		// e.g. their name at the end of the header
		cart->header = INES;
		if (header[9] & 0xFE) { // 9th byte should only contain a 1 or 0
			cart->header = BAD_INES;  // excludes nes files w/ data in the unused bytes
		}
		if ((header[7] & 0x0C) == 0x08) {
			cart->header = NES_2;
		}
	}

	if (cart->header == HEADERLESS) {
		fprintf(stderr, "Error: unrecognised header, requires an offline database.\n");
		fclose(rom);
		return 8;
	}

	// attempt to clean a bad header file
	if (!memcmp(&header[7], "DiskDude!", 9)) {
		printf("Badly formatted iNES header!! Zeroing out tag in header\n");
		memset(&header[7], 0, 9);
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
	log_cart_info(cart, filename, cpu, ppu, &header[0]);

	return 0;
}

static void print_header(Cartridge* cart, uint8_t* header_bytes)
{
	printf("Header bytes: ");
	if (cart->header != HEADERLESS) {
		for (int i = 0; i < 16; i++) {
			printf("%.2X ", header_bytes[i]);
		}
	} else {
		printf(" N/A");
	}
	printf("\n");
}

static void log_cart_info(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu, uint8_t* header_bytes)
{
	printf("Cart: %s\n", filename);
	printf("Mapper number %d\n", cpu->cpu_mapper_io->mapper_number);

	printf("Header Format: ");
	switch (cart->header) {
	case (HEADERLESS):
		printf("Headerless\n");
		break;
	case (BAD_INES):
		printf("iNES (badly formatted)\n");
		break;
	case (INES):
		printf("iNES\n");
		break;
	case (NES_2):
		printf("NES2.0\n");
		break;
	}

	print_header(cart, header_bytes);

	if (cart->video_mode == PAL) {
		printf("PAL\n");
	} else if (cart->video_mode == NTSC) {
		printf("NTSC\n");
	}

	printf("PRG ROM size: %d KiB\n", cart->prg_rom.size / (KiB));
	printf("PRG RAM (WRAM) size: %d KiB\n", cart->prg_ram.size / (KiB));
	printf("CHR ROM size: %d KiB\n", cart->chr.rom_size / (KiB));
	printf("CHR RAM (VRAM) size: %d KiB\n", cart->chr.ram_size / (KiB));

	printf("Mirroring mode: ");
	if (ppu->mirroring == 1) {
		printf("Vertical\n");
	} else if (ppu->mirroring == 0) {
		printf("Horizontal\n");
	} else if (ppu->mirroring == 4) {
		printf("4-screen\n");
	}
}
