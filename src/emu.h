#ifndef __NES_EMU__
#define __NES_EMU__

#include "extern_structs.h"

void ppu_cpu_ratio(Cpu6502* cpu, Ppu2C02* ppu, Display* nes_screen, bool no_logging);
void emu_usuage(const char* program_name);
int main(int argc, char** argv);

#endif /* __NES_EMU__ */
