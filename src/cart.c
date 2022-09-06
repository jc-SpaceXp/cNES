#include "cart.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// iNES/NES2.0 common header masks and results (for 6th byte of header)
#define NES2_IDENTIFIER_MASK       0x0CU
#define PPU_4_SCREEN_MASK          0x08U
#define PPU_VERT_MIRROR_MASK       0x01U
#define NON_VOLATILE_MEM_MASK      0x02U
#define TRAINER_MASK               0x04U
#define NES2_IDENTIFIER            0x08U

// iNES header masks
#define INES_BAD_HEADER_9_MASK     0xFEU   // 9th byte should only contain a 1 or 0
#define INES_PAL_MASK              0x01U   // If the result of the mask is zero it is a NTSC system

// NES2.0 header masks and results
#define NES2_PRG_ROM_MSB_MASK      0x0FU   // upper 4 bits of 12 bit value
#define NES2_CHR_ROM_MSB_MASK      0xF0U   // upper 4 bits of 12 bit value
#define NES2_PRG_ROM_EXPONENT_SIZE 0x0FU
#define NES2_CHR_ROM_EXPONENT_SIZE 0xF0U
#define NES2_MULT_VAL_MASK         0x03U
#define NES2_EXPONENT_VAL_MASK     0xC0U
#define NES2_REGION_MASK           0x03U   // If the result of the mask is zero it is a NTSC system
#define NES2_PAL_REGION            0x01U
#define VOLATILE_RAM_SHIFT_MASK    0x0FU   // represents non-volatile shift counts for CHR and PRG RAM

Cartridge* cart_init(void)
{
	Cartridge* cart = malloc(sizeof(Cartridge));
	if (!cart) {
		fprintf(stderr, "Failed to allocate memory for Cartridge\n");
		return cart; // NULL pointer
	}

	cart->header = HEADERLESS;
	cart->trainer.size = 0;
	cart->non_volatile_mem = false;  // used for "battery-backed" ROMS e.g. Legend of Zelda

	return cart;
}

static uint16_t concat_lsb_and_msb_to_16_bit_val(const uint8_t lsb, const uint8_t msb);
static void log_cart_info(const Cartridge* cart, const char* filename, const Cpu6502* cpu, const Ppu2C02* ppu, const uint8_t* header_bytes);

int parse_nes_cart_file(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu)
{
	uint8_t header[16];
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

	// minimum file size is a headerless .nes file w/ only a 16 KiB PRG ROM
	if (file_size < (16 * KiB - 1)) {
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
		if (header[9] & INES_BAD_HEADER_9_MASK) { // 9th byte should only contain a 1 or 0
			cart->header = BAD_INES;  // excludes nes files w/ data in the unused bytes
		}
		if ((header[7] & NES2_IDENTIFIER_MASK) == NES2_IDENTIFIER) {
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

	/* parsing header, assume iNES format */
	cart->prg_rom.size = 16 * (KiB) * header[4];
	cart->chr.rom_size = 8  * (KiB) * header[5]; // Pattern table data (if any)
	cart->chr.ram_size = 8  * (KiB) * !header[5];
	mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4); // upper-half byte represents the mapper byte number from header bytes 6 and 7
	cart->prg_ram.size = 8  * (KiB) * header[8];
	if (!cart->prg_ram.size) {
		// a zero value implies 8 K for compatability
		// depends on the mapper on whether there is PRG RAM or not
		cart->prg_ram.size = 8  * (KiB);
	}
	cpu->cpu_mapper_io->mapper_number = mapper;

	/* Byte 6, common for both iNES and NES2.0 */
	if (header[6] & PPU_4_SCREEN_MASK) {
		ppu->mirroring = 4;
	} else if (header[6] & PPU_VERT_MIRROR_MASK) {
		ppu->mirroring = 1;
	} else if (!(header[6] & PPU_VERT_MIRROR_MASK)) {
		ppu->mirroring = 0;
	}

	if (header[6] & NON_VOLATILE_MEM_MASK) { // "Battery bit"
		cart->non_volatile_mem = true;
	}

	if (header[6] & TRAINER_MASK) {
		cart->trainer.size = 512;
	}

	/* Byte 9 */
	if (header[9] & INES_PAL_MASK) {
		cart->video_mode = PAL;
	} else {
		cart->video_mode = NTSC;
	}

	if (cart->header == NES_2) {
		cart->prg_rom.size = 16 * (KiB) * concat_lsb_and_msb_to_16_bit_val(header[4], header[9] & NES2_PRG_ROM_MSB_MASK);
		cart->chr.rom_size = 8  * (KiB) * concat_lsb_and_msb_to_16_bit_val(header[5], header[9] & NES2_CHR_ROM_MSB_MASK);

		// Calculate exponent of PRG ROM if necessary
		if ((header[9] & NES2_PRG_ROM_MSB_MASK) == NES2_PRG_ROM_EXPONENT_SIZE) {
			unsigned multiplier = (header[4] & NES2_MULT_VAL_MASK) * 2 + 1;
			cart->prg_rom.size = (1 << (header[4] & NES2_EXPONENT_VAL_MASK)) * multiplier;
		}

		// Calculate exponent of CHR ROM if necessary
		if ((header[9] & NES2_CHR_ROM_MSB_MASK) == NES2_CHR_ROM_EXPONENT_SIZE) {
			unsigned multiplier = (header[5] & NES2_MULT_VAL_MASK) * 2 + 1;
			cart->chr.rom_size = (1 << (header[5] & NES2_EXPONENT_VAL_MASK)) * multiplier;
		}
		// Byte 6 is the same as before
		cpu->cpu_mapper_io->mapper_number |= (header[8] & 0x0F) << 8; // an extra 4 bits are added to the mapper number
		// will add submapper number later
		// Byte 9, already processed for the ROM sizes
		cart->prg_ram.size = 0;
		unsigned shift_count = header[10] & VOLATILE_RAM_SHIFT_MASK;
		if (shift_count) {  cart->prg_ram.size = 64 << shift_count; }

		cart->chr.ram_size = 0;
		shift_count = header[11] & VOLATILE_RAM_SHIFT_MASK;
		if (shift_count) {  cart->chr.ram_size = 64 << shift_count; }

		// so far only processing PAL/NTSC
		cart->video_mode = NTSC;
		if ((header[12] & NES2_REGION_MASK) == NES2_PAL_REGION) {
			cart->video_mode = PAL;
		}
	}

	log_cart_info(cart, filename, cpu, ppu, &header[0]);

	/* Load trainer into member variable */
	if (cart->trainer.size) {
		cart->trainer.data = malloc(cart->trainer.size);
		if (!cart->trainer.data) {
			fclose(rom);
			return 8;
		}
		fread(cart->trainer.data, 1, 512, rom); // size is always 512 bytes if present
	}

	/* Loading data into PRG_ROM */
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


// lsb/msb is least/most significant byte
static inline uint16_t concat_lsb_and_msb_to_16_bit_val(const uint8_t lsb, const uint8_t msb)
{
	return ((msb << 8) | lsb);
}

static void print_header(const Cartridge* cart, const uint8_t* header_bytes)
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

static inline void print_yes_or_no(const int check_non_zero)
{
	if (check_non_zero) {
		printf("Yes\n");
	} else {
		printf("No\n");
	}
}

static void log_cart_info(const Cartridge* cart, const char* filename, const Cpu6502* cpu, const Ppu2C02* ppu, const uint8_t* header_bytes)
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
	printf("PRG RAM (WRAM) size: %d KiB (Some mappers have no PRG RAM, for INES header the value maybe ignored)\n", cart->prg_ram.size / (KiB));
	printf("CHR ROM size: %d KiB\n", cart->chr.rom_size / (KiB));
	printf("CHR RAM (VRAM) size: %d KiB\n", cart->chr.ram_size / (KiB));
	printf("Trainer: ");
	print_yes_or_no(cart->trainer.size);
	printf("Battery/Non-volatile Memory: ");
	print_yes_or_no(cart->non_volatile_mem);

	printf("Mirroring mode: ");
	if (ppu->mirroring == 1) {
		printf("Vertical\n");
	} else if (ppu->mirroring == 0) {
		printf("Horizontal\n");
	} else if (ppu->mirroring == 4) {
		printf("4-screen\n");
	}
}
