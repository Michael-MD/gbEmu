#include "GB.hpp"
#include <chrono>
#include <thread>

GB::GB(std::string gbFilename) : gbInternal(nullptr)
{
	createWindow();

	startGame(gbFilename);

	// ============== Start Game Loop ==============
	gameLoop();
}

GB::GB() : gbInternal(nullptr)
{
	createWindow();

	// ============== Start Game Loop ==============
	gameLoop();
}

void GB::createWindow()
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

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, GridWidth, GridHeight);

		// Setup controllers if any
		int nControllers = SDL_NumJoysticks();

		for (int i = 0; i < nControllers; ++i) {
			if (SDL_IsGameController(i)) {
				SDL_GameController* controller = SDL_GameControllerOpen(i);
			}
		}
	}
	else
	{
		// TODO: throw error, couldn't initialize SDL2
	}
}

GB::~GB()
{
	if (gbInternal != nullptr)
	{
		delete gbInternal;
	}
}

void GB::startGame(std::string gbFilename)
{
	// If this is the first game to start up then
	// initialize everything for the first time.
	if (gbInternal == nullptr)
	{
		gbInternal = new GBInternal(gbFilename);

		// Setup audio
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
		spec.userdata = &(gbInternal->apu);
		device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

		if (device == 0) {
			// Audio device not turned on
			return;
		}

		SDL_PauseAudioDevice(device, 0); // Start playing audio
	}
	else
	{
		SDL_PauseAudioDevice(device, 1); // Stop playing audio
		std::this_thread::sleep_for(std::chrono::microseconds(100));

		delete gbInternal;
		gbInternal = new GBInternal(gbFilename);
		spec.userdata = &(gbInternal->apu);

		std::this_thread::sleep_for(std::chrono::microseconds(100));
		SDL_PauseAudioDevice(device, 0); // Start playing audio

	}
}

void GB::gameLoop()
{
		Uint32 a, b, delta;
		b = SDL_GetTicks();

		while (IsRunning)
		{
			a = SDL_GetTicks();
			delta = a - b;

			if (delta > 1000 / 60.0)
			{
				handleEvents();
				update();
				render();

				b = a;
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
	case SDL_DROPFILE:
		startGame(event.drop.file);
		break;
	}

	// If no game is running then 
	// don't check for controller inputs.
	if (gbInternal == nullptr)
	{
		return;
	}

	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym)
		{
		case SDLK_RIGHT:
			gbInternal->ButtonState[0] &= ~0b0001;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_LEFT:
			gbInternal->ButtonState[0] &= ~0b0010;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_UP:
			gbInternal->ButtonState[0] &= ~0b0100;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_DOWN:
			gbInternal->ButtonState[0] &= ~0b1000;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_v:	// A
			gbInternal->ButtonState[1] &= ~0b0001;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_c:	// B
			gbInternal->ButtonState[1] &= ~0b0010;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_z:	// SELECT
			gbInternal->ButtonState[1] &= ~0b0100;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDLK_x:	// START
			gbInternal->ButtonState[1] &= ~0b1000;
			gbInternal->IE->PNegEdge = 1;
			break;
		}
		break;
	case SDL_KEYUP:
		switch (event.key.keysym.sym)
		{
		case SDLK_RIGHT:
			gbInternal->ButtonState[0] |= 0b0001;
			break;
		case SDLK_LEFT:
			gbInternal->ButtonState[0] |= 0b0010;
			break;
		case SDLK_UP:
			gbInternal->ButtonState[0] |= 0b0100;
			break;
		case SDLK_DOWN:
			gbInternal->ButtonState[0] |= 0b1000;
			break;
		case SDLK_v:	// A
			gbInternal->ButtonState[1] |= 0b0001;
			break;
		case SDLK_c:	// B
			gbInternal->ButtonState[1] |= 0b0010;
			break;
		case SDLK_z:	// SELECT
			gbInternal->ButtonState[1] |= 0b0100;
			break;
		case SDLK_x:	// START
			gbInternal->ButtonState[1] |= 0b1000;
			break;
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		// Handle controller button down events
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			gbInternal->ButtonState[0] &= ~0b0001;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			gbInternal->ButtonState[0] &= ~0b0010;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			gbInternal->ButtonState[0] &= ~0b0100;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			gbInternal->ButtonState[0] &= ~0b1000;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_A:	// A
			gbInternal->ButtonState[1] &= ~0b0001;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_B:	// B
			gbInternal->ButtonState[1] &= ~0b0010;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:	// SELECT
			gbInternal->ButtonState[1] &= ~0b0100;
			gbInternal->IE->PNegEdge = 1;
			break;
		case SDL_CONTROLLER_BUTTON_START:	// START
			gbInternal->ButtonState[1] &= ~0b1000;
			gbInternal->IE->PNegEdge = 1;
			break;
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		// Handle controller button up events
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			gbInternal->ButtonState[0] |= 0b0001;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			gbInternal->ButtonState[0] |= 0b0010;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			gbInternal->ButtonState[0] |= 0b0100;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			gbInternal->ButtonState[0] |= 0b1000;
			break;
		case SDL_CONTROLLER_BUTTON_A:	// A
			gbInternal->ButtonState[1] |= 0b0001;
			break;
		case SDL_CONTROLLER_BUTTON_B:	// B
			gbInternal->ButtonState[1] |= 0b0010;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:	// SELECT
			gbInternal->ButtonState[1] |= 0b0100;
			break;
		case SDL_CONTROLLER_BUTTON_START:	// START
			gbInternal->ButtonState[1] |= 0b1000;
			break;
		}
		break;
	default:
		break;
	}
}

void GB::update()
{
	if (gbInternal == nullptr)
	{
		return;
	}

	SDL_UpdateTexture(texture, NULL, gbInternal->ppu.DotMatrix, GridWidth * sizeof(uint32_t));
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
