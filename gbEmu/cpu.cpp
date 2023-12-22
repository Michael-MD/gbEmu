#include "cpu.hpp"
#include "Bus.hpp"

CPU::CPU()
{
	PC = 0x100;

	// TODO: Internal Checks

	// ============== Initilizes Internal Registers and Memory ============== 

	SP = 0xFFFE;
	F = 0xB0;
	BC = 0X0013;
	DE = 0X00D8;
	HL = 0X014D;
	SP = 0XFFFE;

	bus->write(0xFF05, 0x00);  // TIMA
	bus->write(0xFF06, 0x00);  // TMA
	bus->write(0xFF07, 0x00);  // TAC
	bus->write(0xFF10, 0x80);  // NR10
	bus->write(0xFF11, 0xBF);  // NR11
	bus->write(0xFF12, 0xF3);  // NR12
	bus->write(0xFF14, 0xBF);  // NR14
	bus->write(0xFF16, 0x3F);  // NR21
	bus->write(0xFF17, 0x00);  // NR22
	bus->write(0xFF19, 0xBF);  // NR24
	bus->write(0xFF1A, 0x7F);  // NR30
	bus->write(0xFF1B, 0xFF);  // NR31
	bus->write(0xFF1C, 0x9F);  // NR32
	bus->write(0xFF1E, 0xBF);  // NR33
	bus->write(0xFF20, 0xFF);  // NR41
	bus->write(0xFF21, 0x00);  // NR42
	bus->write(0xFF22, 0x00);  // NR43
	bus->write(0xFF23, 0xBF);  // NR30
	bus->write(0xFF24, 0x77);  // NR50
	bus->write(0xFF25, 0xF3);  // NR51
	bus->write(0xFF26, 0xF1);  // NR52
	bus->write(0xFF40, 0x91);  // LCDC
	bus->write(0xFF42, 0x00);  // SCY
	bus->write(0xFF43, 0x00);  // SCX
	bus->write(0xFF45, 0x00);  // LYC
	bus->write(0xFF47, 0xFC);  // BGP
	bus->write(0xFF48, 0xFF);  // OBP0
	bus->write(0xFF49, 0xFF);  // OBP1
	bus->write(0xFF4A, 0x00);  // WY
	bus->write(0xFF4B, 0x00);  // WX
	bus->write(0xFFFF, 0x00);  // IE
}


inline uint8_t& CPU::GPR(uint8_t i)
{
	switch (i)
	{
	case 0b111:
		return A;
	case 0b000:
		return B;
	case 0b001:
		return C;
	case 0b010:
		return D;
	case 0b011:
		return E;
	case 0b100:
		return H;
	case 0b101:
		return L;
	}
}

uint8_t CPU::ld()
{
	// ============ 8-bit load ============

	if (Op3 == 0b01)
	{
		if (Op1 == 0b110)	// 01 r 110
		{
			// LD r, (HL) (r <- HL)
			GPR(Op2) = bus->read(HL);;
			return 2;
		}
		else if (Op2 == 0b110) // 01 110 r
		{
			// LD (HL),r ((HL) <- r)
			bus->write(HL, GPR(Op1));
			return 2;
		}
		else // 01 r r’
		{
			// LD r,r’ (r <- r')
			GPR(Op2) = GPR(Op3);
			return 1;
		}
	}

	if (Op3 == 0b00 && Op1 == 0b110) // 00 r 110
	{
		// LD r,n (r <- n)
		GPR(Op2) = bus->read(PC++);
	}
	
	uint8_t HI, LO;

	switch (Opcode)
	{
	case(0b00'110'110):
		// LD (HL), n ((HL) <- n)
		bus->write(HL, bus->read(PC++));
		return 3;
	case(0b00'001'010):
		// LD A, (BC) (A <- (BC))
		A = bus->read(BC);
		return 2;
	case(0b00'011'010):
		// LD A, (DE)  (A <- (DE))
		A = bus->read(DE);
		return 2;
	case(0b11'110'010):
		// LD A, (C) (A <- (0xFF00 + C))
		A = bus->read(0xFF00 + C);
		return 2;
	case(0b11'100'010):
		// LD (C), A ((0xFF00H+C) <- A)
		bus->write(0xFF00 + C, A);
		return 2;
	case(0b11'110'000):
		// LD A, (n) (A <- (n))
		A = bus->read(0xFF00 + bus->read(PC++));
		return 3;
	case(0b11'100'000):
		// LD (n), A ((n) <- A)
		bus->write(0xFF00 + bus->read(PC++), A);
		return 3;
	case(0b11'111'010):
		// LD A, (nn)(A <- (nn))
		LO = bus->read(PC++);
		HI = bus->read(PC++);
		A = (HI << 8) | LO;
		return 4;
	case(0b11'101'010):
		// LD (nn), A ((nn) <- A)
		LO = bus->read(PC++);
		HI = bus->read(PC++);
		bus->write((HI << 8) | LO, A);
		return 4;
	case(0b00'101'010):
		// LD A, (HLI) (A <- (HL), HL <- HL + 1)
		A = bus->read(HL++);
		return 2;
	case(0b00'111'010):
		// LD A, (HLD) (A <- (HL), HL <- HL1)
		A = bus->read(HL--);
		return 2;
	case(0b00'000'010):
		// LD (BC), A ((BC) <- A)
		bus->write(BC, A);
		return 2;
	case(0b00'010'010):
		// LD (DE), A ((DE) <- A)
		bus->write(DE, A);
		return 2;
	case(0b00'100'010):
		// LD (HLI), A ((HL) <- A HL <- HL + 1)
		bus->write(HL++, A);
		return 2;
	case(0b00'110'010):
		// LD (HLD), A ((HL) <- A, HL <- HL1)
		bus->write(HL--, A);
		return 2;
	}
}

