#ifndef __CART__
#define __CART__

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */

#include <stdint.h>

#include "extern_structs.h"
#include "mappers.h"

/* FLAG 9 in iNES header */
typedef enum {
	NTSC = 0,
	PAL = 1,
} VideoType;

/* Struct for the different banks of memory present in a ROM i.e CHR_RAM
 * Data represents the actual data present in the bank and size represents
 * how large this data is in KiB.
 */
typedef struct {
	uint8_t* data;
	uint32_t size;
} Memory;

typedef struct Cartridge {
	VideoType video_mode;
	Memory prg_rom;  // Program ROM, sent to CPU
	Memory prg_ram;  // Program RAM, sent to CPU
	Memory chr_rom;  // Sprite and background pattern tables, sent to PPU
} Cartridge;


/* Read .nes file data [cartidge data] into CPU and PPU, whilst also choosing 
 * the correct mapper (nested call).
 */
int load_cart(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2A03* ppu);

#endif /* __CART__ */
