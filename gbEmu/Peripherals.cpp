#include"GB.hpp"

void GB::gameLoop()
{
	int ScreenWidth = 300;
	int ScreenHeight = GB_SCREEN_RATIO * ScreenWidth;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow("gbEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight, 0);
		if (window)
		{
			//throw;
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer)
		{
			//throw ;
		}

		unsigned int a, b = 0, delta, i = 0;
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, ppu.GridWidth, ppu.GridHeight);

		while (IsRunning)
		{
			a = SDL_GetTicks();
			delta = a - b;

			// FOR DEBUGGING

			handleEvents();
			//if (delta > 1000 / (4.19e6 * 4.))	// 4.19 * 4 MHz
			{
				b = a;

				// Clock System
				clock();

				i++;
				//if (i % 280'000 == 0)	// Update Screen at ~60Hz
				//if(i % 28000 == 0)	// Update Screen at ~60Hz
				/*{
					update();
					render();
				}*/
			}

		}

	}
}

void GB::handleEvents()
{
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type)
	{
	case SDL_QUIT:
		IsRunning = false;
		break;
	case SDL_KEYDOWN:


	default:
		break;
	}
}

void GB::update()
{
	SDL_UpdateTexture(texture, NULL, ppu.DotMatrix, ppu.GridWidth * sizeof(uint32_t));
}

void GB::render()
{
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void GB::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
