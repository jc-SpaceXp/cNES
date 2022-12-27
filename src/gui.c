#include "gui.h"

#include <stdlib.h>
#include <stdio.h>


Sdl2Display* sdl2_display_allocator(void)
{
	Sdl2Display* cnes_screen = malloc(sizeof(Sdl2Display));
	if (!cnes_screen) {
		fprintf(stderr, "Failed to allocate enough memory for Sdl2Display struct\n");
	}

	return cnes_screen; // either valid or NULL
}

int screen_init(Sdl2Display* cnes_screen, const char* window_name
               , const unsigned int width, const unsigned int height
               , int scale_factor)
{
	int error_code = 0;
	cnes_screen->window = NULL;
	cnes_screen->renderer = NULL;
	cnes_screen->framebuffer = NULL;

	cnes_screen->window = SDL_CreateWindow(window_name
	                                      , SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
	                                      , width * scale_factor
	                                      , height * scale_factor, SDL_WINDOW_SHOWN);
	if (cnes_screen->window == NULL) {
		fprintf(stderr, "SDL failed to create window: %s\n", SDL_GetError());
		error_code = 1;
	}

	cnes_screen->window_id = SDL_GetWindowID(cnes_screen->window);

	cnes_screen->renderer = SDL_CreateRenderer(cnes_screen->window, -1
	                                          , SDL_RENDERER_ACCELERATED
	                                            | SDL_RENDERER_PRESENTVSYNC);
	if (cnes_screen->renderer == NULL) {
		fprintf(stderr, "SDL failed to create 2D renderer for the window: %s\n", SDL_GetError());
		error_code = 1;
	}

	cnes_screen->framebuffer = SDL_CreateTexture(cnes_screen->renderer, SDL_PIXELFORMAT_ARGB8888
	                                            , SDL_TEXTUREACCESS_STREAMING
	                                            , width, height);
	if (cnes_screen->framebuffer == NULL) {
		fprintf(stderr, "SDL failed to create a texture for the window: %s\n", SDL_GetError());
		error_code = 1;
	}

	// Apply scaling factor
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest") == SDL_FALSE) {
		fprintf(stderr, "SDL failed to set a scaling method/quality for the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	// preserve original aspect ratio
	if (SDL_RenderSetLogicalSize(cnes_screen->renderer, width, height)) {
		fprintf(stderr, "SDL failed to set a new resolution for the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	if (SDL_RenderSetScale(cnes_screen->renderer, scale_factor, scale_factor)) {
		fprintf(stderr, "SDL failed to apply a scaling factor to the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	return error_code;
}

void screen_clear(Sdl2Display* cnes_screen)
{
	SDL_DestroyRenderer(cnes_screen->renderer);
	SDL_DestroyWindow(cnes_screen->window);
	SDL_Quit();
}

void draw_pixels(uint32_t* pixels, const unsigned int width, Sdl2Display* cnes_screen)
{
	// Only render if object exists
	if (cnes_screen->window) {
		SDL_UpdateTexture(cnes_screen->framebuffer, NULL, pixels, width * sizeof(uint32_t));

		SDL_RenderClear(cnes_screen->renderer);
		SDL_RenderCopy(cnes_screen->renderer, cnes_screen->framebuffer, NULL, NULL);
		SDL_RenderPresent(cnes_screen->renderer);
	}
}
