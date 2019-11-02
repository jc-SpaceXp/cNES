#ifndef __NES_EMU__
#define __NES_EMU__

#include "extern_structs.h"

void ppu_cpu_ratio(Cpu6502* CPU, PPU_Struct* PPU, Display* nes_screen);
int main(void);

#endif /* __NES_EMU__ */
