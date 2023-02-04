#ifndef __CART__
#define __CART__

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */

#include "cart_fwd.h"
#include "cpu_fwd.h"
#include "ppu_fwd.h"
#include "mappers.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum HeaderFormat {
	HEADERLESS,
	BAD_INES,
	INES,
	NES_2,
} HeaderFormat;

typedef enum VideoType {
	NTSC = 0,
	PAL = 1,
} VideoType;

struct Memory {
	uint8_t* data;
	uint32_t size;
};

struct ChrMemory {
	uint8_t* data;
	uint32_t rom_size; // either CHR RAM or CHR ROM is used
	uint32_t ram_size; // .. . one of these should hold 0
};

struct Cartridge {
	VideoType video_mode;
	HeaderFormat header;
	Memory prg_rom; // Program ROM, data sent to CPU
	Memory prg_ram; // Program RAM, data sent to CPU
	Memory trainer; // Trainer data (depends on mapper if used or not)
	ChrMemory chr; // CHR data, sprite and background pattern tables sent to PPU
	bool non_volatile_mem; // battery and other types of non-volatile memory
};

/* Read .nes file data [cartidge data] into CPU and PPU, whilst also choosing 
 * the correct mapper (nested call).
 */
Cartridge* cart_allocator(void);
int cart_init(Cartridge* cart);
int parse_nes_cart_file(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu);

#endif /* __CART__ */
