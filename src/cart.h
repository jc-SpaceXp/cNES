#ifndef __CART__
#define __CART__

#define KiB (1024)
#include <stdint.h>
#include "mappers.h"
#include "ppu.h"

/* setup pointers to RAM for PRG etc like done w/ SP ??? */

/* FLAG 9 in iNES header */
/* tydef union?? */
typedef enum {
	NTSC = 0,
	PAL = 1,
} VideoType;

typedef struct {
	uint8_t *data;
	uint32_t size;
} Memory;

typedef struct Cartridge {
	VideoType video_mode;
	Memory prg_rom, prg_ram, chr_rom;
} Cartridge;


int load_cart(Cartridge* cart, const char *filename, PPU_Struct *p);

#endif /* __CART__ */
