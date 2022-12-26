#ifndef __NES_GUI__
#define __NES_GUI__

#include "extern_structs.h" // includes SDL header
#include <stdint.h>

Sdl2Display* sdl2_display_allocator(void);
int screen_init(Sdl2Display* cnes_screen, const char* window_name, int scale_factor);
void screen_clear(Sdl2Display* cnes_screen);
void draw_pixels(uint32_t* pixels, Sdl2Display* cnes_screen); // Draws frame to screen

#endif /* __NES_GUI__ */
