//#include "Papas_Framework.h"
#include <3ds.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL_theora.h>
//===============================================================================
// Framework is constructed as Global
//Papas::Framework g_framework;
//===============================================================================

int main(int argc, char* argv[]) {
    
	// PapasError ret;
	SDL_SetMainReady();
	// // Initialize our framework. This encapsulates all systems.
	// ret = g_framework.init();
	// ASSERT(ret == PAPAS_OK, "");
	romfsInit();
	
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	// Create a window
	SDL_Window *window = SDL_CreateWindow(
		"SDL2 Renderer Example",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		400, 240, // Screen size for the 3DS
		SDL_WINDOW_SHOWN);

	if (!window)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Create a renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Set the renderer draw color (e.g., blue)
	//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

	int my_video = THR_Load("romfs:/videos/test.ogg", renderer);

	SDL_Texture *video_texture = THR_UpdateVideo(my_video);
	// Main loop flag
	int running = 1;
	SDL_Event event;

	while (aptMainLoop())
	{
		// ret = g_framework.update();
		// ASSERT(ret == PAPAS_OK, "");

		// // Exit loop and begin termination if we get anything than a standard OK return
		// if (ret != PAPAS_OK)
		// 	break;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = 0;
			}
		}

		

		

		
		// Use SDL_RenderCopy to blit the texture normally

		if (THR_IsPlaying(my_video) == 0)
			THR_DestroyVideo(my_video);

		// Present the renderer
		SDL_RenderPresent(renderer);
		// Clear the screen
		//SDL_RenderClear(renderer);
	}
	// Clean up
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	// ret = g_framework.terminate();
	// ASSERT(ret == PAPAS_OK, "");
	romfsExit();

	return 0;
}