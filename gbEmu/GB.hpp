#pragma once
#include "SDL.h"
#include "GBInternal.hpp"
#include <string>

class GB
{
public:
	GB();
	GB(std::string gbFilename);
	~GB();

	GBInternal *gbInternal;

	void startGame(std::string gbFilename);
	void createWindow();
	void gameLoop();
	void handleEvents();
	void update();
	void render();
	void clean();

	bool IsRunning = true;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	const float GB_SCREEN_RATIO = 20 / 18;	// height / width
	SDL_AudioDeviceID device;
	SDL_AudioSpec spec;

	const int GridWidth = 20 * 8;
	const int GridHeight = 18 * 8;
};

