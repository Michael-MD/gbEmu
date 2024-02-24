#include"GB.hpp"

void GB::gameLoop()
{
	int ScreenWidth = 300;
	int ScreenHeight = GB_SCREEN_RATIO * ScreenWidth;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow("gbEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight, 0);
		if (window == NULL)
		{
			return;
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer == NULL)
		{
			return;
		}

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, ppu.GridWidth, ppu.GridHeight);

		// Setup controllers if any
		int nControllers = SDL_NumJoysticks();

		for (int i = 0; i < nControllers; ++i) {
			if (SDL_IsGameController(i)) {
				SDL_GameController* controller = SDL_GameControllerOpen(i);
			}
		}

		// Setup audio
		SDL_AudioSpec spec;
		SDL_zero(spec);
		// The gameboy technically outputs samples at
		// the same rate as the internal oscillator.
		// However this is extremely expensive and a modern
		// sound card probabally cannot keep up so we will go
		// with the nominal 44.1kHz which is above nyquist for 
		// all sounds.
		spec.freq = 44100;
		spec.format = AUDIO_S16SYS;
		spec.channels = 2;
		spec.samples = 512;
		spec.callback = &APU::AudioSample; // We will push our own data
		spec.userdata = &apu;
		device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

		if (device == 0) {
			// Audio device not turned on
			return;
		}

		SDL_PauseAudioDevice(device, 0); // Start playing audio

		Uint32 a, b, delta;
		b = SDL_GetTicks();

		while (IsRunning)
		{
			a = SDL_GetTicks();
			delta = a - b;

			if (delta > 1000 / 60.0)
			{
				//for (int j = 0; j < 70'000; j++)	// ~60Hz
				//{
				//	// Clock System
				//	clock();
				//}

				handleEvents();
				update();
				render();

				b = a;
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
		clean();
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
		case SDLK_v:	// A
			ButtonState[1] &= ~0b0001;
			IE->PNegEdge = 1;
			break;
		case SDLK_c:	// B
			ButtonState[1] &= ~0b0010;
			IE->PNegEdge = 1;
			break;
		case SDLK_z:	// SELECT
			ButtonState[1] &= ~0b0100;
			IE->PNegEdge = 1;
			break;
		case SDLK_x:	// START
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
		case SDLK_v:	// A
			ButtonState[1] |= 0b0001;
			break;
		case SDLK_c:	// B
			ButtonState[1] |= 0b0010;
			break;
		case SDLK_z:	// SELECT
			ButtonState[1] |= 0b0100;
			break;
		case SDLK_x:	// START
			ButtonState[1] |= 0b1000;
			break;
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		// Handle controller button down events
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			ButtonState[0] &= ~0b0001;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			ButtonState[0] &= ~0b0010;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			ButtonState[0] &= ~0b0100;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			ButtonState[0] &= ~0b1000;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_A:	// A
			ButtonState[1] &= ~0b0001;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_B:	// B
			ButtonState[1] &= ~0b0010;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:	// SELECT
			ButtonState[1] &= ~0b0100;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_START:	// START
			ButtonState[1] &= ~0b1000;
			IE->PNegEdge = 1;
			break;
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		// Handle controller button up events
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			ButtonState[0] |= 0b0001;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			ButtonState[0] |= 0b0010;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			ButtonState[0] |= 0b0100;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			ButtonState[0] |= 0b1000;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_A:	// A
			ButtonState[1] |= 0b0001;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_B:	// B
			ButtonState[1] |= 0b0010;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:	// SELECT
			ButtonState[1] |= 0b0100;
			IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_START:	// START
			ButtonState[1] |= 0b1000;
			IE->PNegEdge = 1;
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
	SDL_CloseAudioDevice(device);
	SDL_Quit();
}
