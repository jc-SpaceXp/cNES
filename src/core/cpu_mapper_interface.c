#include "cpu_mapper_interface.h"
#include "cart.h"

#include <stdio.h>
#include <stdlib.h>


CpuMapperShare* cpu_mapper_allocator(void)
{
	CpuMapperShare* cpu_mapper = malloc(sizeof(CpuMapperShare));
	if (!cpu_mapper) {
		fprintf(stderr, "Failed to allocate enough memory for CpuMapperShare\n");
	}

	return cpu_mapper; // either returns valid or NULL pointer
}

int cpu_mapper_init(CpuMapperShare* cpu_mapper, Cartridge* cart)
{
	int return_code = -1;
	// Assign mirrors to cartridge data
	cpu_mapper->prg_rom = &cart->prg_rom;
	cpu_mapper->prg_ram = &cart->prg_ram;
	cpu_mapper->chr_rom = &cart->chr_rom;
	cpu_mapper->chr_ram = &cart->chr_ram;


	cpu_mapper->mapper_number = 0;
	cpu_mapper->prg_rom_bank_size = 0;
	cpu_mapper->chr_bank_size = 0;

	cpu_mapper->prg_low_bank_fixed = false;
	cpu_mapper->prg_high_bank_fixed = false;
	cpu_mapper->enable_prg_ram = false;

	return_code = 0;

	return return_code;
}
