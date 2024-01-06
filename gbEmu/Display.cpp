#include "Display.hpp"
#include "GB.hpp"

void Display::connectGB(GB* gb)
{
	this->gb = gb;

	// Reference Display Registers to Main Registers Block
	LY = gb->RAM + 0xFF44;
	LCDC = reinterpret_cast<LCDCRegister*>(gb->RAM + 0xFF40);
	STAT = reinterpret_cast<STATRegister*>(gb->RAM + 0xFF41);
	SCY = gb->RAM + 0xFF42;
	SCX = gb->RAM + 0xFF43;
	BGP = gb->RAM + 0xFF47;
}

void Display::clock()
{
	// ============= LCD Display ============= 
	//if (nClockCycles % 1890 == 0) // Do Entire Row at Once
	if (gb->nClockCycles % 8 == 0) // Do Entire Row at Once
	{
		// TODO: SCX
		// TODO: Fix indexing into appropriate row

		uint8_t BGStartAddr = LCDC->BGCodeArea ? 0x9C00 : 0x9800;

		// If not in vertical blanking period get pixel data
		if (gb->IF->VerticalBlanking == 0)
		{
			for (int ColBlock = 0; ColBlock < 20; ColBlock++)
			{
				// Get CHR Code
				uint8_t CHRCode = gb->RAM[BGStartAddr + (int)(*LY / 8) * 32 + ColBlock];

				// Find Corresponding Tile
				uint8_t DotDataAddr = (CHRCode < 0x80 ? 0x9000 : 0x8800) + 0x0F * CHRCode;

				// Parse Dot Data
				uint8_t TileLO = gb->RAM[DotDataAddr + 16 * (CHRCode & 0x80) + 0];
				uint8_t TileHI = gb->RAM[DotDataAddr + 16 * (CHRCode & 0x80) + 1];

				// Get pixel Shade for entire row of pixels
				for (int p = 0; p < 8; p++)
				{
					uint8_t PixelPalette = ((TileHI & (1 << p)) >> (p - 1)) | (TileLO & (1 << p)) >> p;

					// Store Result Display Grid
					uint8_t value = ((*BGP >> (PixelPalette * 2)) & 0b11) * 255;
					DotMatrix[*LY][ColBlock * 8 + p][0] = 255;
					DotMatrix[*LY][ColBlock * 8 + p][1] = value;
					DotMatrix[*LY][ColBlock * 8 + p][2] = value;
					DotMatrix[*LY][ColBlock * 8 + p][3] = value;
				}
			}
		}

		(*LY) = ((*LY) + 1) % 154;

		// Check if in Vertical Blanking Region
		if (*LY == 18 * 8 - 1)
		{
			gb->IF->VerticalBlanking = 1;
		}
		else if (*LY == 0)
		{
			gb->IF->VerticalBlanking = 0;
		}

	}


	// TODO: window display
}