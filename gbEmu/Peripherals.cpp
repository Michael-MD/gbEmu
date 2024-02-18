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

			//if (delta > 1000 / (4.19e6 * 4.))	// 4.19 * 4 MHz
			{
				b = a;

				i++;

				if (i % 10'000 == 0)
				{
					handleEvents();
				}

				// Clock System
				clock();

				if (i % 70'000 == 0)	// Update Screen at ~60Hz
				{
					update();
					render();
				}
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
		switch (event.key.keysym.sym)
		{
		case SDLK_RIGHT:
			ButtonState[0] &= ~0b0001;
			IE->PNegEdge = 1;
			break;
		case SDLK_LEFT:
			ButtonState[0] &= ~0b0010;
			IE->PNegEdge = 1;
			break;
		case SDLK_UP:
			ButtonState[0] &= ~0b0100;
			IE->PNegEdge = 1;
			break;
		case SDLK_DOWN:
			ButtonState[0] &= ~0b1000;
			IE->PNegEdge = 1;
			break;
		case SDLK_z:	// A
			ButtonState[1] &= ~0b0001;
			IE->PNegEdge = 1;
			break;
		case SDLK_a:	// B
			ButtonState[1] &= ~0b0010;
			IE->PNegEdge = 1;
			break;
		case SDLK_w:	// SELECT
			ButtonState[1] &= ~0b0100;
			IE->PNegEdge = 1;
			break;
		case SDLK_q:	// START
			ButtonState[1] &= ~0b1000;
			IE->PNegEdge = 1;
			break;
		}
		break;
	case SDL_KEYUP:
		switch (event.key.keysym.sym)
		{
		case SDLK_RIGHT:
			ButtonState[0] |= 0b0001;
			break;
		case SDLK_LEFT:
			ButtonState[0] |= 0b0010;
			break;
		case SDLK_UP:
			ButtonState[0] |= 0b0100;
			break;
		case SDLK_DOWN:
			ButtonState[0] |= 0b1000;
			break;
		case SDLK_z:	// A
			ButtonState[1] |= 0b0001;
			break;
		case SDLK_a:	// B
			ButtonState[1] |= 0b0010;
			break;
		case SDLK_w:	// SELECT
			ButtonState[1] |= 0b0100;
			break;
		case SDLK_q:	// START
			ButtonState[1] |= 0b1000;
			break;
		}
		break;
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
