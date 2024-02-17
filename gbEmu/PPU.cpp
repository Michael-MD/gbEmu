#include "PPU.hpp"
#include "GB.hpp"

#include <algorithm>

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

	OBP0 = gb->RAM + 0xFF48;
	OBP1 = gb->RAM + 0xFF49;

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
			
			gb->IF->LCDC = STAT->Mode10Selection;

			// Check for scanline sprites all in one go
			if (DotsTotal == 0)
			{
				nScanLineObjects = 0;
				// Find objects which cover current pixel to be drawn
				// TODO: Overlap priorities
				for (uint8_t i = 0; i < 40; i++)
				{
					Object* Obj = reinterpret_cast<Object*>(&gb->RAM[0xFE00 + i * 4]);

					// Check if sprite overlaps in y-direction
					if (LCDC->OBJ8x16)	// Sprites 2 tiles tall
					{
						if (Obj->YPos > *LY && Obj->YPos - 16 <= *LY)
						{
							FoundObject = true;
						}
					}
					else
					{
						if (Obj->YPos - 8 > *LY && Obj->YPos - 16 <= *LY)
						{
							FoundObject = true;
						}
					}

					if (FoundObject)
					{
						FoundObject = false;
						ScanLineObjects[nScanLineObjects] = Obj;

						// Maximum number of objects per scanline is 10
						if (++nScanLineObjects == 10)
						{
							break;
						}
					}

				}

				// To more easily determine priorities we order the objects
				// based on x-ccordinate while keeping sprites with the same
				// x-ccordinate in the same order relative to how they were in
				// the OAM.
				std::stable_sort(
					ScanLineObjects,
					ScanLineObjects + nScanLineObjects,
					[](Object* e1, Object* e2) { return e1->XPos < e2->XPos; }
				);
			}

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
			DotsTotal = 0;		// Reset Row Dots Count

			if (*LY == 144)		// Vblank begins at 144
			{
				STAT->ModeFlag = 0b01;
				gb->IF->LCDC = STAT->Mode01Selection;
			}

			// Reset window y coordinate
			if (*LY == 153)		// Vblank end at 153
			{
				WLY = 0;
			}

			break;

		case HorizontalBlank:
			DotsRemaining = 456 - DotsTotal;
			STAT->ModeFlag = 0b00;
			gb->IF->LCDC = STAT->Mode00Selection;

			// If window was visible on the current scanline, then increment WLY;
			if (LCDC->bBG && LCDC->bWindowing && LX >= *WX - 7 && *LY >= *WY)
			{
				WLY++;
			}

			break;
		}
	}
	
	DotsRemaining--;
	FoundObject = false;

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

		// TODO: Priority
		
		if (LCDC->bOBJ)
		{
			// Find objects which cover current pixel to be drawn
			// TODO: Overlap priorities
			for (uint8_t i = 0; i < nScanLineObjects; i++)
			{
				Object* Obj = ScanLineObjects[i];

				// Check if sprite overlaps in x-direction
				if (Obj->XPos > LX && Obj->XPos - 8 <= LX)
				{
					FoundObject = true;
					Obj = Obj;

					// Get tile row from memory
					// Check if sprite is flipped in y-direction
					int TileRow;
					uint8_t TileHeight = LCDC->OBJ8x16 ? 16 : 8;
					if (Obj->YFlip)
					{
						TileRow = (TileHeight - 1) - (*LY - (Obj->YPos - 16));
					}
					else
					{
						TileRow = *LY - (Obj->YPos - 16);
					}

					// Enforced in hardware is the igorance of the LSB of the tile index
					// when the ppu is in 8x16 mode.
					if (LCDC->OBJ8x16)
					{
						Obj->TileIndex &= 0xFE;
					}

					uint8_t TileLO = gb->RAM[0x8000 + Obj->TileIndex * 0x10 + TileRow * 2 + 0];
					uint8_t TileHI = gb->RAM[0x8000 + Obj->TileIndex * 0x10 + TileRow * 2 + 1];

					// Get pixel color or in the case of DMG, the
					// shade of pixel from OBP0 or OBP1 registers.
					uint8_t ObP = Obj->Pallette ? *OBP1 : *OBP0;

					// Check if bit is flipped in x-directions
					uint8_t p;
					if (Obj->XFlip)
					{
						p = (LX - (Obj->XPos - 8)) % 8;
					}
					else
					{
						p = 7 - (LX - (Obj->XPos - 8));
					}

					uint8_t PixelPalette = (((TileHI >> p) & 0x01) << 1) | ((TileLO >> p) & 0x01);

					// Index 0 is always transparent
					if (PixelPalette == 0)
					{
						FoundObject = false;
						ObjectPriorityConflict = false;
					}
					else
					{
						FoundObject = true;
						ObjectPriorityConflict = Obj->Priority;

						Value = (3 - ((ObP >> (PixelPalette * 2)) & 0b11)) * 255 / 3;
					}

					break;

				}
			}

		}
		
		// TODO: colours 1-3 only are drawn over sprite
		
		// Either no object was found or an object was found but the 
		// priority may result in the object pixel not being rendered.
		if(!FoundObject || (FoundObject && ObjectPriorityConflict))
		{
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

				if(PixelPalette != 0 || !ObjectPriorityConflict)
				{
					Value = (3 - (((*BGP) >> (PixelPalette * 2)) & 0b11)) * 255 / 3;
				}
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

				if (PixelPalette != 0 || !ObjectPriorityConflict)
				{
					Value = (3 - (((*BGP) >> (PixelPalette * 2)) & 0b11)) * 255 / 3;
				}
			}
			else
			{
				// If bBG flag of LCDC register is not set, then
				// on all models except the CGB, this means the
				// screen is simply white.
				Value = 255;
			}
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