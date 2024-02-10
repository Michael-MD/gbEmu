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
	
	// Set-up pixel rendering
	Mode = VerticalBlank;
	DotsRemaining = 0;
	DotsTotal = 0;
}

void PPU::clock()
{
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
			if (++(*LY) == 144)		// Vblank begins at 144
			{
				Mode = VerticalBlank;
			}
			else
			{
				Mode = OAMScan;
			}
			break;
		case VerticalBlank:
			(*LY) = ((*LY) + 1) % 154;		// Next Row with wrap around
			if ((*LY) < 144)	// Vblank lines are 144 through 153 inclusive
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
			STAT->Mode10Selection = 1;
			gb->IF->LCDC = 1;

			// Reset vblank flag if previous scan line was vblank
			if(*LY == 0)
				gb->IF->VerticalBlanking = 0;

			break;

		case DrawingPixels:
			DotsRemaining = 172;
			STAT->ModeFlag = 0b11;
			bLineRendered = false;
			break;

		case VerticalBlank:
			DotsRemaining = 456;
			DotsTotal = 0;					// Reset Row Dots Count

			if (*LY == 144)		// Vblank begins at 144
			{
				// Alert CPU that PPU is in vertical blanking period
				gb->IF->VerticalBlanking = 1;
				STAT->Mode01Selection = 1; // Interrupt Selection: Vertical Blanking
				gb->IF->LCDC = 1;
				STAT->ModeFlag = 0b01;
			}
			break;

		case HorizontalBlank:
			DotsRemaining = 456 - DotsTotal;
			STAT->ModeFlag = 0b00;
			STAT->Mode00Selection = 1;
			gb->IF->LCDC = 1;
			break;
		}

		// Checks if scan line has reached value
		// stored in LYC. This can happen any time 
		// if LYC is set.
		STAT->MatchFlag = (*LY) == (*LYC);
		if (STAT->MatchFlag)
		{
			STAT->LYCMatch = 1;	// Interrupt Selection: LY == LYC 
			gb->IF->LCDC = 1;
		}

	}
	
	DotsRemaining--;


	// =========== Draw Pixels ===========
	if (Mode == DrawingPixels)
	{
		// Check if background rendering enabled and line 
		// not already rendered.
		if (LCDC->bBG == 1 && bLineRendered == false)
		{
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

			// Only loop through visible tiles
			for (int LX = 0; LX < 20 * 8; LX++)
			{
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
				uint8_t p = 8 - (LX + *SCX) % 8;

				uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);
				uint8_t Value = (((*BGP) >> (PixelPalette * 2)) & 0b11) * 255 / 4;

				// Place pixel value into dot matrix
				DotMatrix[*LY][LX][0] = 255;
				DotMatrix[*LY][LX][1] = Value;
				DotMatrix[*LY][LX][2] = Value;
				DotMatrix[*LY][LX][3] = Value;
			}

		}
		else if (LCDC->bBG == 0 && bLineRendered == false)
		{
			// If bBG flag of LCDC register is not set, then
			// on all models except the CGB, the means the
			// screen is simply white.
			for (int Tile_i = 0; Tile_i < 20; Tile_i++)
			{
				for (uint8_t p = 0; p < 8; p++)
				{
					uint8_t PixelColumn = Tile_i * 8 + (8 - p - 1);

					DotMatrix[*LY][PixelColumn][0] = 255;
					DotMatrix[*LY][PixelColumn][1] = 255;
					DotMatrix[*LY][PixelColumn][2] = 255;
					DotMatrix[*LY][PixelColumn][3] = 255;
				}
			}
			
		}

		bLineRendered = true;
	}

	
}