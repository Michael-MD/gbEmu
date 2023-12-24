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

	InstructionSet[0b00'000'000] =
	{
		"NOP ()",
		[this]() {

		},
		1
	};
	
	for (int i = 0; i <= 7; i+1 == 0b110 ? i += 2 : i++)
	{
		for (int j = 0; j <= 7; j + 1 == 0b110 ? j += 2 : j++)
		{
			uint8_t r = GPR(i), rp = GPR(j);
			InstructionSet[0x40|(i<<3)|j] =
			{
				"LD r,r’ (r <- r')",
				[this, &r, &rp]() {
					r = rp;
				},
				1
			};
		}
	}

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0x00 | (i << 3) | 0b110] =
		{
			"LD r,n (r <- n)",
			[this, &r]() {
				r = bus->read(PC++);
			},
			2
		};

		InstructionSet[0x00 | (i << 3) | 0b110] =
		{
			"LD r, (HL) (r <- (HL))",
			[this, &r]() {
				r = bus->read(HL);
			},
			2
		};
	}

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b01'110'000 | i] =
		{
			"LD (HL),r ((HL) <- r)",
			[this, &r]() {
				bus->write(HL, r);
			},
			2
		};
	}

	InstructionSet[0b00'110'110] =
	{
		"LD (HL), n ((HL) <- n)",
		[this]() {
			bus->write(HL, bus->read(PC++));
		},
		3
	};

	InstructionSet[0b00'001'010] =
	{
		"LD A, (BC) (A <- (BC))",
		[this]() {
			A = bus->read(BC);
		},
		2
	};

	InstructionSet[0b00'011'010] =
	{
		"LD A, (DE)  (A <- (DE))",
		[this]() {
			A = bus->read(DE);
		},
		2
	};

	InstructionSet[0b11'110'010] =
	{
		"LD A, (C) (A <- (0xFF00 + C))",
		[this]() {
			A = bus->read(0xFF00 + C);
		},
		2
	};

	InstructionSet[0b11'100'010] =
	{
		"LD (C), A ((0xFF00H+C) <- A)",
		[this]() {
			bus->write(0xFF00 + C, A);
		},
		2
	};

	InstructionSet[0b11'110'000] =
	{
		"LD A, (n) (A <- (n))",
		[this]() {
			A = bus->read(0xFF00 + bus->read(PC++));
		},
		3
	};

	InstructionSet[0b11'100'000] =
	{
		"LD (n), A ((n) <- A)",
		[this]() {
			bus->write(0xFF00 + bus->read(PC++), A);
		},
		3
	};

	InstructionSet[0b11'111'010] =
	{
		"LD A, (nn)(A <- (nn))",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			A = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'101'010] =
	{
		"LD (nn), A ((nn) <- A)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			bus->write((HI << 8) | LO, A);
		},
		4
	};

	InstructionSet[0b00'101'010] =
	{
		"LD A, (HLI) (A <- (HL), HL <- HL + 1)",
		[this]() {
			A = bus->read(HL++);
		},
		2
	};

	InstructionSet[0b00'111'010] =
	{
		"LD A, (HLD) (A <- (HL), HL <- HL1)",
		[this]() {
			A = bus->read(HL--);
		},
		2
	};

	InstructionSet[0b00'000'010] =
	{
		"LD (BC), A ((BC) <- A)",
		[this]() {
			bus->write(BC, A);
		},
		2
	};

	InstructionSet[0b00'010'010] =
	{
		"LD (DE), A ((DE) <- A)",
		[this]() {
			bus->write(DE, A);
		},
		2
	};

	InstructionSet[0b00'100'010] =
	{
		"LD (HLI), A ((HL) <- A HL <- HL + 1)",
		[this]() {
			bus->write(HL++, A);
		},
		2
	};

	InstructionSet[0b00'110'010] =
	{
		"LD (HLD), A ((HL) <- A, HL <- HL",
		[this]() {
			bus->write(HL--, A);
		},
		2
	};

	InstructionSet[0b00'000'001] =
	{
		"LD dd, nn (BC <- nn)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			BC = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'010'001] =
	{
		"LD dd, nn (DE <- nn)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			DE = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'100'001] =
	{
		"LD dd, nn (HL <- nn)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			HL = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'110'001] =
	{
		"LD dd, nn (SP <- nn)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			SP = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b11'111'001] =
	{
		"LD SP 11 SP, HL (SP <- HL)",
		[this]() {
			SP = HL;
		},
		2
	};

	InstructionSet[0b11'000'101] =
	{
		"PUSH BC ((SP-1) <- BCH (SP - 2) <- BCL SP <- SP - 2)",
		[this]() {
			bus->write(--SP, BC >> 8);
			bus->write(--SP, BC & 0x0F);
		},
		4
	};

	InstructionSet[0b11'010'101] =
	{
		"PUSH DE ((SP-1) <- DEH (SP - 2) <- DEL SP <- SP - 2)",
		[this]() {
			bus->write(--SP, DE >> 8);
			bus->write(--SP, DE & 0x0F);
		},
		4
	};

	InstructionSet[0b11'100'101] =
	{
		"PUSH HL ((SP-1) <- HLH (SP - 2) <- HLL SP <- SP - 2)",
		[this]() {
			bus->write(--SP, HL >> 8);
			bus->write(--SP, HL & 0x0F);
		},
		4
	};

	InstructionSet[0b11'110'101] =
	{
		"PUSH AF ((SP-1) <- AFH (SP - 2) <- AFL SP <- SP - 2)",
		[this]() {
			bus->write(--SP, AF >> 8);
			bus->write(--SP, AF & 0x0F);
		},
		4
	};

	InstructionSet[0b11'000'001] =
	{
		"POP BC (BCL <- (SP) BCH <- (SP + 1) SP <- SP + 2)",
		[this]() {
			C = bus->read(SP++);
			B = bus->read(SP++);
		},
		3
	};

	InstructionSet[0b11'010'001] =
	{
		"POP DE (DEL <- (SP) DEH <- (SP + 1) SP <- SP + 2)",
		[this]() {
			E = bus->read(SP++);
			D = bus->read(SP++);
		},
		3
	};

	InstructionSet[0b11'100'001] =
	{
		"POP HL (HLL <- (SP) HLH <- (SP + 1) SP <- SP + 2)",
		[this]() {
			L = bus->read(SP++);
			H = bus->read(SP++);
		},
		3
	};

	InstructionSet[0b11'110'001] =
	{
		"POP AF (AFL <- (SP) AFH <- (SP + 1) SP <- SP + 2)",
		[this]() {
			F = bus->read(SP++);
			A = bus->read(SP++);
		},
		3
	};

	InstructionSet[0b11'111'000] =
	{
		"LDHL SP, e (HL <- SP+e)",
		[this]() {
			int8_t e = bus->read(PC++);
			uint32_t tmp = SP + e;
			HL = tmp & 0xFFFF;
			Z = 0;
			N = 0;
			HC = (((SP & 0x000F) + e) >> 8) != 0;
			CY = (tmp >> 16) != 0;
		},
		3
	};

	InstructionSet[0b00'001'000] =
	{
		"LD (nn), SP ((nn) <- SPL (nn + 1) <- SPH)",
		[this]() {
			uint8_t LO, HI;
			LO = bus->read(PC++);
			HI = bus->read(PC++);
			uint16_t M = (HI << 8) | LO;
			bus->write(M++, SP & 0x00FF);
			bus->write(M, SP & 0xFF00);
		},
		5
	};

	InstructionSet[0b11'000'110] =
	{
		"ADD A,n (A <- A+n)",
		[this]() {
			uint8_t n = bus->read(PC++);
			uint16_t tmp = A + n;
			uint8_t tmp2 = ((A & 0xF) + (n & 0xF)) >> 4;

			HC = tmp2 != 0;
			CY = (tmp >> 8) != 0;
			A = tmp;
			Z = A == 0;
			N = 0;
		},
		2
	};

	InstructionSet[0b10'000'110] =
	{
		"ADD A, (HL) (A <- A+(HL))",
		[this]() {
			A += bus->read(HL);
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'001'000 | i] =
		{
			"ADC A, r (A <- A+s+CY)",
			[this, &r]() {
				uint16_t tmp = A + r + CY;
				uint8_t tmp2 = ((A & 0xF) + (r & 0xF) + CY) >> 4;

				HC = tmp2 != 0;
				CY = (tmp >> 8) != 0;
				A = tmp;
				Z = A == 0;
				N = 0;
			},
			1
		};
	}

	InstructionSet[0b11'001'110] =
	{
		" ADC A, n (A <- A+n+CY)",
		[this]() {
			uint8_t n = bus->read(PC++);
			uint16_t tmp = A + n + CY;
			uint8_t tmp2 = ((A & 0xF) + (n & 0xF) + CY) >> 4;

			HC = tmp2 != 0;
			CY = (tmp >> 8) != 0;
			A = tmp;
			Z = A == 0;
			N = 0;
		},
		2
	};

	InstructionSet[0b10'001'110] =
	{
		" ADC A, (HL) (A <- A+n+CY)",
		[this]() {
			uint8_t M = bus->read(HL);
			uint16_t tmp = A + M + CY;
			uint8_t tmp2 = ((A & 0xF) + (M & 0xF) + CY) >> 4;

			HC = tmp2 != 0;
			CY = (tmp >> 8) != 0;
			A = tmp;
			Z = A == 0;
			N = 0;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'010'000 | i] =
		{
			"SUB r (A <- A-r)",
			[this, &r]() {
				HC = (A & 0xF) < (r & 0xF);
				CY = A < r;

				A -= r;
				Z = A == 0;
				N = 1;
			},
			1
		};
	}

	InstructionSet[0b11'010'110] =
	{
		"SUB n ( A <- A-n)",
		[this]() {
			uint8_t n = bus->read(PC++);

			HC = (A & 0xF) < (n & 0xF);
			CY = A < n;

			A -= n;
			Z = A == 0;
			N = 1;
		},
		2
	};

	InstructionSet[0b10'010'110] =
	{
		"SUB (HL) ( A <- A-(HL))",
		[this]() {
			uint8_t M = bus->read(HL);

			HC = (A & 0xF) < (M & 0xF);
			CY = A < M;

			A -= M;
			Z = A == 0;
			N = 1;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'011'000 | i] =
		{
			"SBC A, r (A <- A-r-CY)",
			[this, &r]() {
				HC = (A & 0xF) < ((r + 1) & 0xF);
				CY = A < (r + 1);

				A -= (r + 1);
				Z = A == 0;
				N = 1;
			},
			1
		};
	}

	InstructionSet[0b11'011'110] =
	{
		"SBC A, n (A <- A - n - CY)",
		[this]() {
			uint8_t n = bus->read(PC++);

			HC = (A & 0xF) < ((n + 1) & 0xF);
			CY = A < (n + 1);

			A -= (n + 1);
			Z = A == 0;
			N = 1;
		},
		2
	};

	InstructionSet[0b10'011'110] =
	{
		"SBC A, (HL) (A <- A - (HL) - CY)",
		[this]() {
			uint8_t M = bus->read(HL);

			HC = (A & 0xF) < ((M + 1) & 0xF);
			CY = A < (M + 1);

			A -= (M + 1);
			Z = A == 0;
			N = 1;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'100'000 | i] =
		{
			"AND r (A & r)",
			[this, &r]() {
				A &= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1
		};
	}

	InstructionSet[0b11'100'110] =
	{
		"AND n (A & n)",
		[this]() {
			A &= bus->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'100'110] =
	{
		"AND (HL) (A & (HL))",
		[this]() {
			A &= bus->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'110'000 | i] =
		{
			"OR r (A | r)",
			[this, &r]() {
				A |= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1
		};
	}

	InstructionSet[0b11'110'110] =
	{
		"OR n (A | n)",
		[this]() {
			A |= bus->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'110'110] =
	{
		"OR (HL) (A | (HL))",
		[this]() {
			A |= bus->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'101'000 | i] =
		{
			"XOR r (A ^ r)",
			[this, &r]() {
				A ^= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1
		};
	}

	InstructionSet[0b11'101'110] =
	{
		"XOR n (A ^ n)",
		[this]() {
			A ^= bus->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'101'110] =
	{
		"XOR (HL) (A ^ (HL))",
		[this]() {
			A ^= bus->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b10'101'000 | i] =
		{
			"CP r (A == r)",
			[this, &r]() {
				Z = A == r;
				HC = (A & 0xF) < (r & 0xF);
				N = 1;
				CY = A < r;
			},
			1
		};
	}

	InstructionSet[0b11'111'110] =
	{
		"CP n (A == n)",
		[this]() {
			uint8_t n = bus->read(PC++);
			Z = A == n;
			HC = (A & 0xF) < (n & 0xF);
			N = 1;
			CY = A < n;
		},
		1
	};

	InstructionSet[0b11'111'110] =
	{
		"CP (HL) (A == (HL))",
		[this]() {
			uint8_t M = bus->read(HL);
			Z = A == M;
			HC = (A & 0xF) < (M & 0xF);
			N = 1;
			CY = A < M;
		},
		1
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b00'000'100 | (i << 3)] =
		{
			"INC r (r <- r+1)",
			[this, &r]() {
				HC = ((r & 0xF) + (1 & 0xF)) >> 4;
				N = 0;
				r += 1;
				Z = r == 0;
			},
			1
		};
	}

	InstructionSet[0b00'110'100] =
	{
		"INC (HL) ((HL) <- (HL)+1)",
		[this]() {
			uint8_t M = bus->read(HL);
			HC = ((M & 0xF) + (1 & 0xF)) >> 4;
			N = 0;
			M += 1;
			Z = M == 0;
			bus->write(HL, M);
		},
		3
	};

	for (int i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		uint8_t r = GPR(i);
		InstructionSet[0b00'000'101 | (i << 3)] =
		{
			"DEC r (r <- r-1)",
			[this, &r]() {
				HC = (r & 0xF) < (1 & 0xF);
				N = 1;
				r -= 1;
				Z = r == 0;
			},
			1
		};
	}

	InstructionSet[0b00'110'101] =
	{
		"DEC M (M <- M-1)",
		[this]() {
			uint8_t M = bus->read(HL);
			HC = (M & 0xF) < (1 & 0xF);
			N = 1;
			M -= 1;
			Z = M == 0;
			bus->write(HL, M);
		},
		3
	};
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

void CPU::clock()
{
	
}
