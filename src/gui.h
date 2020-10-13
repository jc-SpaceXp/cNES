#ifndef __NES_GUI__
#define __NES_GUI__

#include "extern_structs.h" // includes SDL header
#include <stdint.h>

Display* screen_init(void);
void screen_clear(Display* nes);
//int SDL_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **framebuffer);
void draw_pixels(uint32_t* pixels, Display* nes); // Draws frame to screen
/*
void draw_texture(uint32_t *pixels, Display *nes); // Draws frame to screen
void draw_display(Display *nes); // Draws frame to screen
*/

#endif /* __NES_GUI__ */
