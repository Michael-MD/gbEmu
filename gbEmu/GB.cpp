#include "GB.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

GB::GB(std::string gbFilename)
{
	nClockCycles = 0;

	// Connect SM83 to remainder of system
	cpu.gb = this;

	// Insert Cartridge
	Cartridge cart(gbFilename);

	// ============== Initilizes Registers ==============
	*TMA = 0x00;
	*TAC = 0x00;
	*P1 = 0x00;
	*LY = 0;
	*SCY = 0x00;
	*SCX = 0x00;
	*IE = 0x00;

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
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, GridWidth, GridHeight);

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
	SDL_UpdateTexture(texture, NULL, Display, GridWidth * sizeof(Uint32));
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

	// Increment Divider register at 8.192kHz.
	if (nClockCycles % 0xFF == 0 && nClockCycles % 4 == 0)
		(*Div)++;

	// ============= LCD Display ============= 
	//if (nClockCycles % 1890 == 0) // Do Entire Row at Once
	if (nClockCycles % 8 == 0) // Do Entire Row at Once
	{
		// TODO: SCX
		// TODO: Fix indexing into appropriate row

		uint8_t BGStartAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;

		// If not in vertical blanking period get pixel data
		if (IF->VerticalBlanking == 0)
		{
			for (int ColBlock = 0; ColBlock < 20; ColBlock++)
			{
				// Get CHR Code
				uint8_t CHRCode = RAM[BGStartAddr + (int)(*LY / 8) * 32 + ColBlock];

				// Find Corresponding Tile
				uint8_t DotDataAddr = (CHRCode < 0x80 ? 0x9000 : 0x8800) + 0x0F * CHRCode;

				// Parse Dot Data
				uint8_t TileLO = RAM[DotDataAddr + 16 * (CHRCode & 0x80) + 0];
				uint8_t TileHI = RAM[DotDataAddr + 16 * (CHRCode & 0x80) + 1];

				// Get pixel Shade for entire row of pixels
				for (int p = 0; p < 8; p++)
				{
					uint8_t PixelPalette = ((TileHI & (1 << p)) >> (p - 1)) | (TileLO & (1 << p)) >> p;

					// Store Result Display Grid
					uint8_t value = ((*BGP >> (PixelPalette * 2)) & 0b11) * 255;
					Display[*LY][ColBlock * 8 + p][0] = 255;
					Display[*LY][ColBlock * 8 + p][1] = value;
					Display[*LY][ColBlock * 8 + p][2] = value;
					Display[*LY][ColBlock * 8 + p][3] = value;
				}
			}
		}

		(*LY) = ((*LY) + 1) % 154;

		// Check if in Vertical Blanking Region
		if (*LY == 18 * 8 - 1)
		{
			IF->VerticalBlanking = 1;
		}
		else if (*LY == 0)
		{
			IF->VerticalBlanking = 0;
		}
		
	}


	// TODO: window display

}

uint8_t GB::read(uint16_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000)		// 16kB ROM bank #0 
	{

	}
	else if (addr >= 0x4000 && addr < 0x8000)	// 16kB switchable ROM bank
	{

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
	if (addr >= 0xC000 && addr < 0xFE00)	// 8kB Internal RAM
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
