#pragma once
#include <cstdint>
#include "SM83.hpp"
#include "Cartridge.hpp"
#include "PPU.hpp"
#include "Timer.hpp"
#include "DMA.hpp"
#include <string>
#include "SDL.h"
#include "APU.hpp"

class GB
{
public:
	GB(std::string gbFilename);
	~GB();

	SM83 cpu;
	Cartridge* cart;
	PPU ppu;
	Timer timer;
	DMA dma;
	APU apu;

	SDL_AudioDeviceID device;

	uint32_t nClockCycles;

	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t data);
	void clock();

	// RAM for Memory Mapping
	uint8_t RAM[0xFFFF + 1];

	// ========= Memory Mapped Registers ========= 

	// Controller Data
	union
	{
		struct
		{
			uint8_t P10 : 1;
			uint8_t P11 : 1;
			uint8_t P12 : 1;
			uint8_t P13 : 1;
			uint8_t P14 : 1;
			uint8_t P15 : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};

	} *P1 = reinterpret_cast<decltype(P1)>(RAM + 0xFF00);

	// Used to keep track of which buttons have
	// been pressed. ButtonState[0] stores the D-pad.
	// ButtonState[1] stores the other buttons (SsBA).
	// If no button is pressed then lower bit 
	// is 0xFF. Note, this switch is an active low.
	uint8_t ButtonState[2] = { 0xFF , 0xFF };

	// ================== Serial Transfer ================== 
	std::string SerialOut;

	uint8_t* SB = RAM + 0xFF01;	// Serial transfer data

	// Serial transfer control
	union
	{
		struct
		{
			uint8_t TransferEnable : 1;
			uint8_t ClockSpeed : 1;
			uint8_t ClockSelect : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *SC = reinterpret_cast<decltype(SC)>(RAM + 0xFF02);

	// ================== Interrupt Registers ==================

	// Interrupt Request
	union
	{
		struct
		{
			uint8_t VerticalBlanking : 1;
			uint8_t LCDC : 1;
			uint8_t TimerOverflow : 1;
			uint8_t SerialIOTransferCompletion : 1;
			uint8_t PNegEdge : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
			reg |= 0xE0;	// First 3 bits are always set
		};

	} *IF = reinterpret_cast<decltype(IF)>(RAM + 0xFF0F);

	// Interrupt Enable
	union
	{
		struct
		{
			uint8_t VerticalBlanking : 1;
			uint8_t LCDC : 1;
			uint8_t TimerOverflow : 1;
			uint8_t SerialIOTransferCompletion : 1;
			uint8_t PNegEdge : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};

	} *IE = reinterpret_cast<decltype(IE)>(RAM + 0xFFFF);

private:
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
};