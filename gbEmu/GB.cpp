#include "GB.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

GB::GB(std::string gbFilename)
{
	nClockCycles = 0;

	// Connect SM83 to remainder of system
	cpu.connectGB(this);

	// Insert Cartridge
	cart = new Cartridge(gbFilename);

	// Initialize display
	disp.connectGB(this);

	// ============== Initilizes Registers ==============
	// CPU Internal Registers
	cpu.BC = 0x0013;
	cpu.DE = 0x00D8;
	cpu.HL = 0x014D;
	cpu.SP = 0xFFFE;
	cpu.PC = 0x0100;

	*TIMA = 0x00;
	*TMA = 0x00;
	*TAC = 0x00;
	*NR10 = 0x80;
	*NR11 = 0xBF;
	*NR12 = 0xF3;
	*NR14 = 0xBF;
	*NR21 = 0x3F;
	*NR22 = 0x00;
	*NR24 = 0xBF;
	*NR30 = 0x7F;
	*NR31 = 0xFF;
	*NR32 = 0x9F;
	*NR33 = 0xBF;
	*NR41 = 0xFF;
	*NR42 = 0x00;
	*NR43 = 0x00;
	*NR44 = 0xBF;
	*NR50 = 0x77;
	*NR51 = 0xF3;
	*NR52 = cart->Header->SuperGB ? 0xF0 : 0xF1;

	*disp.LCDC = 0x91;
	*P1 = 0x00;
	*disp.LY = 0;
	*disp.SCY = 0x00;
	*disp.SCX = 0x00;
	*disp.BGP = 0xFC;
	*IE = 0x00;

	// TODO: Finish remaining initialization

	// ============== Start Game Loop ==============
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
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, disp.GridWidth, disp.GridHeight);

		while (bIsRunning)
		{
			a = SDL_GetTicks();
			delta = a - b;

			handleEvents();
			if (delta > 1000 / (4.19e6 * 4.))	// 4.19 * 4 MHz
			//if (delta > 1000 / (10'000))	// 4.19 * 4 MHz
			{
				b = a;

				// Clock System
				clock();

				i++;
				if(i % 280'000 == 0)	// Update Screen at ~60Hz
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
		bIsRunning = false;
		break;
	case SDL_KEYDOWN:


	default:
		break;
	}
}

void GB::update()
{
	SDL_UpdateTexture(texture, NULL, disp.DotMatrix, disp.GridWidth * sizeof(uint32_t));
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


void GB::clock()
{
	// The system should be clocked in DMG mode
	// at 4f where f=4.1943MHz i.e. machine cycles.

	nClockCycles++;

	cpu.clock();
	disp.clock();

	// Increment Divider register at 8.192kHz.
	if (nClockCycles % 0xFF == 0 && nClockCycles % 4 == 0)
		(*Div)++;

}

uint8_t GB::read(uint16_t addr)
{
	if (addr < 0x8000)		// Cartridge
	{
		return cart->read(addr);
	}
	else if (addr >= 0x8000 && addr < 0xA000)	// 8kB Video RAM
	{

	}
	else if (addr >= 0xA000 && addr < 0xC000)	// 8kB switchable RAM bank
	{

	}
	else if (addr >= 0xC000 && addr < 0xFE00)	// 8kB Internal RAM
	{
		
	}
	else if (addr >= 0xFE00 && addr < 0xFEA0)	// Sprite Attrib Memory (OAM)
	{

	}
	else if (addr >= 0xFEA0 && addr < 0xFF00)	// Empty but unusable for I/O
	{

	}
	else if (addr >= 0xFF00 && addr < 0xFF4C)	// I/O ports
	{

	}
	else if (addr >= 0xFF4C && addr < 0xFF80)	// Empty but unusable for I/O
	{

	}
	else if (addr >= 0xFF80 && addr < 0xFFFF)	// Internal RAM
	{

	}
	else if (addr == 0xFFFF)					// Interrupt Enable Register
	{

	}

	return RAM[addr];
}

void GB::write(uint16_t addr, uint8_t data)
{
	if (addr < 0x8000)		// Cartridge
	{
		cart->write(addr, data);
	}
	else if (addr >= 0xC000 && addr < 0xFE00)	// 8kB Internal RAM
	{
		// Echo 8kB Internal RAM

		if (addr >= 0xC000 && addr < 0xE000)
		{
			RAM[addr] = data;
			RAM[0xE000 + addr & 0xC000] = data;
		}
		else
		{
			RAM[0xC000 + addr & 0xE000] = data;
			RAM[addr] = data;
		}
	}
	else if (addr == 0xFF04)	// Divider Register
	{
		// Writing any value to Divider register sets it to 0x00.
		*Div = 0x00;
	}
	else
	{
		RAM[addr] = data;
	}
}
