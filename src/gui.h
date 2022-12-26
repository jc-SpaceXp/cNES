#ifndef __NES_GUI__
#define __NES_GUI__

#include "extern_structs.h" // includes SDL header
#include <stdint.h>

Sdl2Display* screen_init(int scale_factor);
void screen_clear(Sdl2Display* nes);
void draw_pixels(uint32_t* pixels, Sdl2Display* nes); // Draws frame to screen

#endif /* __NES_GUI__ */
