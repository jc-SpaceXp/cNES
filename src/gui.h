#ifndef __NES_GUI__
#define __NES_GUI__

#include "extern_structs.h" // includes SDL header
#include <stdint.h>

Display* screen_init(int scale_factor);
void screen_clear(Display* nes);
void draw_pixels(uint32_t* pixels, Display* nes); // Draws frame to screen

#endif /* __NES_GUI__ */
