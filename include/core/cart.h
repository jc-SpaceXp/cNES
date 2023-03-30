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

struct CartMemory {
	uint8_t* data;
	uint32_t size;
};

struct Cartridge {
	VideoType video_mode;
	HeaderFormat header;
	CartMemory prg_rom; // Program ROM, data sent to CPU
	CartMemory prg_ram; // Program RAM, data sent to CPU
	CartMemory trainer; // Trainer data (depends on mapper if used or not)
	CartMemory chr_rom; // CHR data, sprite and background pattern tables sent to PPU
	CartMemory chr_ram;
	bool non_volatile_mem; // battery and other types of non-volatile memory
};

/* Read .nes file data [cartidge data] into CPU and PPU, whilst also choosing 
 * the correct mapper (nested call).
 */
Cartridge* cart_allocator(void);
int cart_init(Cartridge* cart);
int parse_nes_cart_file(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu);

#endif /* __CART__ */
