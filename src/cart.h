#ifndef __CART__
#define __CART__

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */

#include <stdint.h>

#include "extern_structs.h"
#include "mappers.h"

/* Read .nes file data [cartidge data] into CPU and PPU, whilst also choosing 
 * the correct mapper (nested call).
 */
Cartridge* cart_init(void);
int load_cart(Cartridge* cart, const char* filename, Cpu6502* cpu, Ppu2C02* ppu);

#endif /* __CART__ */
