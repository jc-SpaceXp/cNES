#ifndef __CPU_MAPPER_INTERFACE__
#define __CPU_MAPPER_INTERFACE__

#include "cpu_mapper_interface_fwd.h"
#include "cart_fwd.h"

#include <stdbool.h>


// Shared mapper/cpu struct
struct CpuMapperShare {
	// Mirror data from Cart struct
	CartMemory* prg_rom;
	CartMemory* prg_ram;
	CartMemory* chr_rom;
	CartMemory* chr_ram;
	unsigned mapper_number;

	// currently these are only MMC1 specific (struct may change later)
	unsigned prg_rom_bank_size;
	unsigned chr_bank_size;

	// prg_rom helpers
	bool prg_low_bank_fixed;  // Bank $8000 to $BFFF is fixed
	bool prg_high_bank_fixed; // Bank $C000 to $FFFF is fixed

	bool enable_prg_ram;
};

CpuMapperShare* cpu_mapper_allocator(void);
int cpu_mapper_init(CpuMapperShare* cpu_mapper, Cartridge* cart);


#endif /* __CPU_MAPPER_INTERFACE__ */
