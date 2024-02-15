#include "PPU.hpp"
#include "GB.hpp"

void PPU::connectGB(GB* gb)
{
	this->gb = gb;

	// Reference PPU Registers to Main Registers Block
	LY = gb->RAM + 0xFF44;
	LYC = gb->RAM + 0xFF45;
	LCDC = reinterpret_cast<LCDCRegister*>(gb->RAM + 0xFF40);
	STAT = reinterpret_cast<STATRegister*>(gb->RAM + 0xFF41);
	SCY = gb->RAM + 0xFF42;
	
	SCX = gb->RAM + 0xFF43;
	BGP = gb->RAM + 0xFF47;

	WY = gb->RAM + 0xFF4A;
	WX = gb->RAM + 0xFF4B;

	// Set-up pixel rendering
	Mode = VerticalBlank;
	DotsRemaining = 0;
	DotsTotal = 0;
}

void PPU::setLY(uint8_t v)
{
	*LY = v % 154;

	// Reset vblank flag if previous scan line was vblank
	if (*LY == 0)
	{
		gb->IF->VerticalBlanking = 0;
	}
	else if (*LY == 144)
	{
		// Alert CPU that PPU is in vertical blanking period
		gb->IF->VerticalBlanking = 1;
		gb->IF->LCDC = 1;
	}
}

void PPU::clock()
{
	// Checks if ppu is off
	if (LCDC->bLCDC == 0)
	{
		return;
	}

	// LY == LYC is checked contiuously during OAMScan
	// Checks if scan line has reached value
	// stored in LYC. This can happen any time 
	// if LYC is set.
	bool MatchFlag_tmp = (*LY) == (*LYC);
	if (MatchFlag_tmp)
	{
		if (STAT->MatchFlag != MatchFlag_tmp && STAT->LYCMatch)
		{
			// Triggers only when bit flips.
			gb->IF->LCDC = 1;
		}

		STAT->MatchFlag = 1;
	}
	else
	{
		STAT->MatchFlag = 0;
	}

	// Because mode 3 of rendering
	// has variable duration between 172 
	// and 289 dots, the horizontal blanking
	// period (mode 0) will endure until the 
	// end of the scan line which is always 
	// 456 dots. So we keep track of the total 
	// dots elapsed per scan line to ensure we 
	// know how long the blanking period will be.
	DotsTotal++;

	// if DotsRemaining == 0 then we are at the 
	// end of the current mode and we should
	// proceed to the next.
	if (DotsRemaining == 0)
	{	
		// Determine next mode
		switch (Mode)
		{
		case OAMScan:
			Mode = DrawingPixels;
			break;
		case DrawingPixels:
			Mode = HorizontalBlank;
			break;
		case HorizontalBlank:
			setLY(*LY + 1);
			if (*LY == 144)		// Vblank begins at 144
			{
				Mode = VerticalBlank;
			}
			else
			{
				Mode = OAMScan;
			}
			break;
		case VerticalBlank:
			setLY(*LY + 1);		// Next Row with wrap around
			if (*LY < 144)	// Vblank lines are 144 through 153 inclusive
			{
				Mode = OAMScan;
			}
			else
			{
				Mode = VerticalBlank;
			}
			break;
		}
		
		// TODO: STAT blocking
		switch (Mode)
		{
		case OAMScan:	// Begin drawing pixels
			DotsTotal = 0;					// Reset Row Dots Count

			DotsRemaining = 80;
			STAT->ModeFlag = 0b10;
			
			gb->IF->LCDC = STAT->Mode10Selection != 0;

			break;

		case DrawingPixels:
			DotsRemaining = 160;

			// At the start there are delays in mode 3 due to:
			//		- 12 T-cycles for FIFO
			//		- SCX % 8 since the first few pixels are discarded
			Delay += 12;
			Delay += *SCX % 8;
			DotsRemaining += Delay;

			STAT->ModeFlag = 0b11;
			bLineRendered = false;
			LX = 0;

			break;

		case VerticalBlank:
			DotsRemaining = 456;
			DotsTotal = 0;					// Reset Row Dots Count

			if (*LY == 144)		// Vblank begins at 144
			{
				STAT->ModeFlag = 0b01;
				gb->IF->LCDC = STAT->Mode01Selection != 0;
			}

			if (*LY == 153)		// Vblank end at 153
			{
				WLY = 0;
			}

			break;

		case HorizontalBlank:
			DotsRemaining = 456 - DotsTotal;
			STAT->ModeFlag = 0b00;
			gb->IF->LCDC = STAT->Mode00Selection != 0;

			// If window was visible on the current scanline, then increment WLY;
			if (LCDC->bBG && LCDC->bWindowing && LX >= *WX - 7 && *LY >= *WY)
			{
				WLY++;
			}

			break;
		}
	}
	
	DotsRemaining--;

	switch (Mode)
	{
	case OAMScan:
		break;
	case DrawingPixels:

		if (Delay > 0)
		{
			Delay -= 1;
			break;
		}

		// =========== Draw Pixels ===========
		// Check if window is enabled and current pixel in
		// window's coverage. If not then try drawing a 
		// background pixel, otherwise just display a 
		// white pixel on the monochrome gameboy.
		uint8_t Value;
		// TODO: WX < 7 behaviour
		if (LCDC->bBG && LCDC->bWindowing && LX >= *WX - 7 && *LY >= *WY)
		{
			// ============ Window Display ============ 

			// Get addressing mode for accessing
			// VRAM which is the range 0x8000-0x97FF
			uint16_t BGBaseAddr = LCDC->BGCharData ? 0x8000 : 0x9000;

			// Get base address for character codes for tiles
			// 0: 0x9800 - 0x9BFF
			// 1: 0x9C00 - 0x9FFF
			uint16_t CHRCodesBaseAddr = LCDC->WindowCodeArea ? 0x9C00 : 0x9800;

			// TODO: Midframe behaviour
			uint8_t LineY = WLY;
			uint8_t LineX = LX - (*WX - 7);

			// Read character code of tile i on line (*LY) / 8
			uint8_t CHRCode = gb->RAM[CHRCodesBaseAddr + (int)(LineY / 8) * 32 + (int)(LineX / 8)];

			// Determine mode by which tile data is located
			// based on unsigned or signed offset from 
			// base address.
			int16_t CHRCodeOffset = LCDC->BGCharData ? (uint8_t)CHRCode : (int8_t)CHRCode;

			// Get tile data, offset from base address and 
			// by the row being rendered of a given tile which 
			// is 8x8. Each row of tile is two bytes.
			int TileRow = LineY % 8;
			uint8_t TileLO = gb->RAM[BGBaseAddr + CHRCodeOffset * 0x10 + TileRow * 2 + 0];
			uint8_t TileHI = gb->RAM[BGBaseAddr + CHRCodeOffset * 0x10 + TileRow * 2 + 1];

			// Get pixel color or in the case of DMG, the
			// shade of pixel from BGP register.
			uint8_t p = 7 - LineX % 8;

			uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);
			Value = (3 - (((*BGP) >> (PixelPalette * 2)) & 0b11)) * 255 / 3;
		}
		else if (LCDC->bBG)
		{
			// ============ Background Display ============ 

			// Get addressing mode for accessing
			// VRAM which is the range 0x8000-0x97FF
			uint16_t BGBaseAddr = LCDC->BGCharData ? 0x8000 : 0x9000;

			// Get base address for character codes for tiles
			// 0: 0x9800 - 0x9BFF
			// 1: 0x9C00 - 0x9FFF
			uint16_t CHRCodesBaseAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;

			// The GB has the capability of scrolling the screen, the offset
			// from the top left corner is specified through SCX and SCY.
			// TODO: Midframe behaviour
			uint8_t LineY = (*LY + *SCY) % 256;

			// Read character code of tile i on line (*LY) / 8
			uint8_t CHRCode = gb->RAM[CHRCodesBaseAddr + (int)(LineY / 8) * 32 + (int)(((LX + *SCX) % 256) / 8)];

			// Determine mode by which tile data is located
			// based on unsigned or signed offset from 
			// base address.
			int16_t CHRCodeOffset = LCDC->BGCharData ? (uint8_t)CHRCode : (int8_t)CHRCode;

			// Get tile data, offset from base address and 
			// by the row being rendered of a given tile which 
			// is 8x8. Each row of tile is two bytes.
			int TileRow = LineY % 8;
			uint8_t TileLO = gb->RAM[BGBaseAddr + CHRCodeOffset * 0x10 + TileRow * 2 + 0];
			uint8_t TileHI = gb->RAM[BGBaseAddr + CHRCodeOffset * 0x10 + TileRow * 2 + 1];

			// Get pixel color or in the case of DMG, the
			// shade of pixel from BGP register.
			uint8_t p = 7 - (LX + *SCX) % 8;

			uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);
			Value = (3 - (((*BGP) >> (PixelPalette * 2)) & 0b11)) * 255 / 3;
		}
		//else if (LCDC->bBG == 0)
		else
		{
			// If bBG flag of LCDC register is not set, then
			// on all models except the CGB, this means the
			// screen is simply white.
			Value = 255;
		}

		// Place pixel value into dot matrix
		DotMatrix[*LY][LX][0] = 255;
		DotMatrix[*LY][LX][1] = Value;
		DotMatrix[*LY][LX][2] = Value;
		DotMatrix[*LY][LX][3] = Value;

		LX += 1;
		
		break;
	}

	
}

void PPU::reset()
{
	*SCX = 0x00;
	*SCY = 0x00;

	// TODO: Check this behaviour
	*LY = 0x00;
	Mode = HorizontalBlank;
	DotsRemaining = 456 - 289;
	STAT->ModeFlag = 0b00;
	
	//LCDC->bBG = 0;
}