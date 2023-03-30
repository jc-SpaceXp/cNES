#ifndef __NES_EMU__
#define __NES_EMU__

#include "SDL2/SDL_events.h"
#include "cpu_fwd.h"
#include "ppu_fwd.h"
#include "gui_fwd.h"

#include <stdbool.h>

void ppu_cpu_ratio(Cpu6502* cpu, Ppu2C02* ppu, Sdl2DisplayOutputs* cnes_windows, const bool logging_cpu_instructions);
void emu_usuage(const char* program_name);
void process_player_1_input(SDL_Event e, Cpu6502* cpu);
void process_window_events(SDL_Event e, Sdl2Display* cnes_screen);
int main(int argc, char** argv);

#endif /* __NES_EMU__ */
