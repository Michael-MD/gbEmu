#include "cpu.hpp"
#include "Bus.hpp"


void CPU::clock()
{
	// CPU::clock is called at ~16MHz.

	if(bus->nClockCycles % 4 ==0) // Machine cycles to clock cycles
	{
		// Timer Unit
		if (bus->TAC->Start)
		{
			uint16_t mod;
			switch (bus->TAC->InputClockSelect)
			{
			case 0b00:	// f/2^10 (4.096kHz)
				mod = 0x3FF;
				break;
			case 0b01:	// f/2^4 (262.144kHz)
				mod = 0x03;
				break;
			case 0b10:	// f/2^6 (65.536kHz)
				mod = 0x1F;
				break;
			case 0b11:	// f/2^8 (16.384kHz)
				mod = 0x7F;
				break;
			}

			if (*(bus->TIMA) == 0xFF)
			{
				// Overflow
				*bus->TIMA = *bus->TIMA;
				bus->IF->TimerOverflow = 1;	// Raise Timer Overflow interrupt flag
			}
			else
			{
				if (bus->nClockCycles % mod == 0)
					(*bus->TIMA)++;
			}
		}

		if (cycle == 0)
		{
			// Check for interrupts between intruction fetch cycles
			if (bus->IME)
			{
				// Check if any interrupts have occured
				if ((bus->IE->reg & bus->IF->reg & 0x1F) != 0)
				{
					// Check interrupts in order of precedence

					bus->IME = 0;
					bus->write(--SP, PC >> 8);
					bus->write(--SP, PC & 0x00FF);

					if (bus->IE->VerticalBlanking && bus->IF->VerticalBlanking)
					{
						PC = 0x0040;
					}
					else if (bus->IE->LCDC && bus->IF->LCDC)
					{
						PC = 0x0048;
					}
					else if (bus->IE->TimerOverflow && bus->IF->TimerOverflow)
					{
						PC = 0x0050;
					}
					else if (bus->IE->SerialIOTransferCompletion && bus->IF->SerialIOTransferCompletion)
					{
						PC = 0x0058;
					}
					else if (bus->IE->PNegEdge && bus->IF->PNegEdge)
					{
						PC = 0x0060;
					}
				}
			}

			// Fetch Next Instruction
			uint8_t data = bus->read(PC++);
			cycle += InstructionSet[data].cycles;
			InstructionSet[data].op();
		}
		else
		{
			// Continue Current Instruction Execution
			cycle--;
		}
	}

	// IME reset after interrupt occurs	
}


CPU::CPU()
{
	PC = 0x100;

	// TODO: Internal Checks
 

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

	for (int i = 0; i <= 3; i++)
	{
		uint16_t r = qq(i);

		InstructionSet[0b11'000'101 | (i << 4)] =
		{
			"PUSH qq ((SP-1) <- qqH (SP - 2) <- qqL SP <- SP - 2)",
			[this, &r]() {
				bus->write(--SP, r >> 8);
				bus->write(--SP, r & 0x0F);
			},
			4
		};

		InstructionSet[0b11'000'001 | (i << 4)] =
		{
			"POP qq (qqL <- (SP) qqH <- (SP + 1) SP <- SP + 2)",
			[this, &r]() {
				r = (r & 0xFF00) | bus->read(SP++);
				r = (bus->read(SP++) << 8) | (r & 0x00FF);
			},
			3
		};
	}

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
		"DEC (HL) ((HL) <- (HL)-1)",
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

	InstructionSet[0b00'001'001] =
	{
		"ADD HL,BC (HL <- HL+BC)",
		[this]() {
			HC = (((HL & 0xFFF) + (BC & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)HL+(uint32_t)BC) >> 16) != 0;
			HL += BC;
			N = 0;
		},
		2
	};

	InstructionSet[0b00'011'001] =
	{
		"ADD HL,DE (HL <- HL+DE)",
		[this]() {
			HC = (((HL & 0xFFF) + (DE & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)HL+(uint32_t)DE) >> 16) != 0;
			HL += DE;
			N = 0;
		},
		2
	};

	InstructionSet[0b00'101'001] =
	{
		"ADD HL,HL (HL <- HL+HL)",
		[this]() {
			HC = (((HL & 0xFFF) + (HL & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)HL+(uint32_t)HL) >> 16) != 0;
			HL += HL;
			N = 0;
		},
		2
	};

	InstructionSet[0b00'111'001] =
	{
		"ADD HL,SP (HL <- HL+SP)",
		[this]() {
			HC = (((HL & 0xFFF) + (SP & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)HL+(uint32_t)SP) >> 16) != 0;
			HL += SP;
			N = 0;
		},
		2
	};

	InstructionSet[0b11'101'000] =
	{
		"ADD SP,e (SP <- SP+e)",
		[this]() {
			int8_t e = bus->read(PC++);
			HC = (((SP & 0xFFF) + (e & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)SP + (uint32_t)e) >> 16) != 0;
			SP += e;
			N = 0;
			Z = 0;
		},
		4
	};
	
	for (int i = 0; i <= 3; i++)
	{
		uint16_t r = ss(i);

		InstructionSet[0b00'000'011 | (i << 4)] =
		{
			"INC ss (ss <- ss + 1)",
			[this, &r]() {
				r += 1;
			},
			2
		};

		InstructionSet[0b00'001'011 | (i << 4)] =
		{
			"DEC ss (ss <- ss - 1)",
			[this, &r]() {
				r -= 1;
			},
			2
		};
	}

	InstructionSet[0b00'000'111] =
	{
		"RLCA",
		[this]() {
			CY = A >> 7;
			A <<= 1;
			A |= CY;
			HC = 0;
			N = 0;
			Z = 0;
		},
		1
	};

	InstructionSet[0b00'010'111] =
	{
		"RLA",
		[this]() {
			uint8_t tmp = A >> 7;
			A <<= 1;
			A |= CY;
			CY = tmp;
			HC = 0;
			N = 0;
			Z = 0;
		},
		1
	};

	InstructionSet[0b00'001'111] =
	{
		"RRCA",
		[this]() {
			CY = A & 0x01;
			A >>= 1;
			A |= CY << 7;
			HC = 0;
			N = 0;
			Z = 0;
		},
		1
	};

	InstructionSet[0b00'011'111] =
	{
		"RRA",
		[this]() {
			uint8_t tmp = A & 0x01;
			A >>= 1;
			A |= CY;
			CY = tmp;
			HC = 0;
			N = 0;
			Z = 0;
		},
		1
	};

	InstructionSet[0b11'001'011] =
	{
		"Shift",
		[this]() {
			uint8_t D = bus->read(PC++);
			uint8_t raddr = D & 0x03;
			uint8_t opH = D >> 3;

			switch (opH)
			{
			case 0b00'000:
				if (raddr != 0b110)
				{
					// RLC r
					uint8_t& r = GPR(raddr);
					CY = r >> 7;
					r <<= 1;
					r |= CY;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// RLC (HL)
					uint8_t M = bus->read(HL);
					CY = M >> 7;
					M <<= 1;
					M |= CY;
					bus->write(HL, M);
					N = 0;
					HC = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'010:
				if (raddr != 0b110)
				{
					// RL r
					uint8_t& r = GPR(raddr);
					uint8_t tmp = r >> 7;
					r <<= 1;
					r |= CY;
					CY = tmp;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// RL (HL)
					uint8_t M = bus->read(HL);
					uint8_t tmp = M >> 7;
					M <<= 1;
					M |= CY;
					bus->write(HL, M);
					CY = tmp;
					N = 0;
					HC = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'001:
				if (raddr != 0b110)
				{
					// RRC r
					uint8_t& r = GPR(raddr);
					CY = r & 0x01;
					r >>= 1;
					r |= CY << 7;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// RRC (HL)
					uint8_t M = bus->read(HL);
					CY = M & 0x01;
					M >>= 1;
					M |= CY << 7;
					bus->write(HL, M);
					N = 0;
					HC = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'011:
				if (raddr != 0b110)
				{
					// RR r
					uint8_t& r = GPR(raddr);
					uint8_t tmp = r & 0x01;
					r >>= 1;
					r |= CY << 7;
					CY = tmp;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// RR (HL)
					uint8_t M = bus->read(HL);
					uint8_t tmp = M & 0x01;
					M >>= 1;
					M |= CY << 7;
					CY = tmp;
					bus->write(HL, M);
					N = 0;
					HC = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'100:
				if (raddr != 0b110)
				{
					// SLA r
					uint8_t& r = GPR(raddr);
					CY = r >> 7;
					r <<= 1;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// SLA (HL)
					uint8_t M = bus->read(HL);
					CY = M >> 7;
					M <<= 1;
					HC = 0;
					N = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'101:
				if (raddr != 0b110)
				{
					// SRA r
					uint8_t& r = GPR(raddr);
					uint8_t tmp = r & 0x80;
					CY = r & 0x01;
					r >>= 1;
					r |= tmp;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// SRA (HL)
					uint8_t M = bus->read(HL);
					uint8_t tmp = M & 0x80;
					CY = M & 0x01;
					M >>= 1;
					M |= tmp;
					HC = 0;
					N = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'111:
				if (raddr != 0b110)
				{
					// SRL r
					uint8_t& r = GPR(raddr);
					CY = r & 0x01;
					r >>= 1;
					HC = 0;
					N = 0;
					Z = r == 0;
				}
				else
				{
					// SRL (HL)
					uint8_t M = bus->read(HL);
					CY = M & 0x01;
					M >>= 1;
					HC = 0;
					N = 0;
					Z = M == 0;
					cycle += 2;
				}
				break;
			case 0b00'110:
				if (raddr != 0b110)
				{
					// SWAP r
					uint8_t& r = GPR(raddr);
					uint8_t rH = r >> 4;
					r = (r << 4) | rH;
				}
				else
				{
					// SWAP (HL)
					uint8_t M = bus->read(HL);
					uint8_t MH = M >> 4;
					M = (M << 4) | MH;
					cycle += 2;
				}
				break;
			}
		},
		2
	};

	InstructionSet[0b11'001'011] =
	{
		"BIT",
		[this]() {
			uint8_t D = bus->read(PC++);
			uint8_t raddr = D & 0x03;
			uint8_t& r = GPR(raddr);
			uint8_t b = (D >> 3) & 0x03;
			uint8_t opH = D >> 6;

			switch (opH)
			{
			case 0b01:
				if (raddr != 0b110)
				{
					// BIT b,r (Z <- ~rb)
					Z = (~r >> b) & 0b1;
					HC = 1;
					N = 0;
				}
				else
				{
					// BIT b,(HL) (Z <- ~(HL)b)
					uint8_t M = bus->read(HL);
					Z = (~M >> b) & 0b1;
					HC = 1;
					N = 0;

					cycle += 1;
				}
				break;
			case 0b11:
				if (raddr != 0b110)
				{
					// SET b,r (rb <- 1)
					r |= (1 << b);
				}
				else
				{
					// SET b,(HL) ((HL)b <- 1)
					uint8_t M = bus->read(HL);
					M |= (1 << b);
					bus->write(HL, M);

					cycle += 2;
				}
				break;
			case 0b10:
				if (raddr != 0b110)
				{
					// RES b,r (rb <- 0)
					r &= ~(1 << b);
				}
				else
				{
					// RES b,(HL) ((HL)b <- 0)
					uint8_t M = bus->read(HL);
					M &= ~(1 << b);
					bus->write(HL, M);

					cycle += 2;
				}
				break;
			}
		},
		2
	};
	
	InstructionSet[0b11'000'011] =
	{
		"JP nn (PC <- nn)",
		[this]() {
			uint8_t LO = bus->read(PC++);
			uint8_t HI = bus->read(PC++);
			PC = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'000'010] =
	{
		"JP ~Z, nn (If ~Z: PC <- nn)",
		[this]() {
			if (Z == 0)
			{
				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;
				cycle += 1;
			}
			else
			{
				PC += 2;
			}
		},
		3
	};

	InstructionSet[0b11'001'010] =
	{
		"JP Z, nn (If Z: PC <- nn)",
		[this]() {
			if (Z == 1)
			{
				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;
				cycle += 1;
			}
			else
			{
				PC += 2;
			}
		},
		3
	};

	InstructionSet[0b11'010'010] =
	{
		"JP ~CY, nn (If ~CY: PC <- nn)",
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;
				cycle += 1;
			}
			else
			{
				PC += 2;
			}
		},
		3
	};

	InstructionSet[0b11'011'010] =
	{
		"JP CY, nn (If CY: PC <- nn)",
		[this]() {
			if (CY == 1)
			{
				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;
				cycle += 1;
			}
			else
			{
				PC += 2;
			}
		},
		3
	};

	InstructionSet[0b00'011'000] =
	{
		"JR e (PC <- PC+e)",
		[this]() {
			int8_t e = bus->read(PC++);
			PC = e + 2;
		},
		3
	};

	InstructionSet[0b00'100'000] =
	{
		"JR ~Z, e (If ~Z: PC <- PC+e)",
		[this]() {
			if (Z == 0)
			{
				int8_t e = bus->read(PC++);
				PC = e + 2;
				cycle++;
			}
		},
		2
	};

	InstructionSet[0b00'101'000] =
	{
		"JR Z, e (If Z: PC <- PC+e)",
		[this]() {
			if (Z == 1)
			{
				int8_t e = bus->read(PC++);
				PC = e + 2;
				cycle++;
			}
		},
		2
	};

	InstructionSet[0b00'110'000] =
	{
		"JR ~CY, e (If ~CY: PC <- PC+e)",
		[this]() {
			if (CY == 0)
			{
				int8_t e = bus->read(PC++);
				PC = e + 2;
				cycle++;
			}
		},
		2
	};

	InstructionSet[0b00'111'000] =
	{
		"JR CY, e (If CY: PC <- PC+e)",
		[this]() {
			if (CY == 1)
			{
				int8_t e = bus->read(PC++);
				PC = e + 2;
				cycle++;
			}
		},
		2
	};

	InstructionSet[0b11'101'001] =
	{
		"JP (HL) (PC <- HL)",
		[this]() {
			PC = HL;
		},
		1
	};

	InstructionSet[0b11'001'101] =
	{
		"CALL nn",
		[this]() {
			bus->write(--SP, PC >> 8);
			bus->write(--SP, PC & 0x00FF);

			uint8_t LO = bus->read(PC++);
			uint8_t HI = bus->read(PC++);
			PC = (HI << 8) | LO;
		},
		6
	};

	InstructionSet[0b11'000'100] =
	{
		"CALL cc, ~Z",
		[this]() {
			if (Z == 0)
			{
				bus->write(--SP, PC >> 8);
				bus->write(--SP, PC & 0x00FF);

				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'001'100] =
	{
		"CALL cc, Z",
		[this]() {
			if (Z == 1)
			{
				bus->write(--SP, PC >> 8);
				bus->write(--SP, PC & 0x00FF);

				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'010'100] =
	{
		"CALL cc, ~CY",
		[this]() {
			if (CY == 0)
			{
				bus->write(--SP, PC >> 8);
				bus->write(--SP, PC & 0x00FF);

				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'011'100] =
	{
		"CALL cc, CY",
		[this]() {
			if (CY == 1)
			{
				bus->write(--SP, PC >> 8);
				bus->write(--SP, PC & 0x00FF);

				uint8_t LO = bus->read(PC++);
				uint8_t HI = bus->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'001'001] =
	{
		"RET",
		[this]() {
			uint8_t LO = bus->read(SP++);
			uint8_t HI = bus->read(SP++);
			PC = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'011'001] =
	{
		"RETI",
		[this]() {
			uint8_t LO = bus->read(SP++);
			uint8_t HI = bus->read(SP++);
			PC = (HI << 8) | LO;
			bus->IME = 1; // TODO: Double Check
		},
		4
	};

	InstructionSet[0b11'000'000] =
	{
		"RET ~Z",
		[this]() {
			if (Z == 0)
			{
				uint8_t LO = bus->read(SP++);
				uint8_t HI = bus->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'001'000] =
	{
		"RET Z",
		[this]() {
			if (Z == 1)
			{
				uint8_t LO = bus->read(SP++);
				uint8_t HI = bus->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'010'000] =
	{
		"RET ~CY",
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = bus->read(SP++);
				uint8_t HI = bus->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'011'000] =
	{
		"RET CY",
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = bus->read(SP++);
				uint8_t HI = bus->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	for(int t = 0; t <= 7; t++)
	{
		InstructionSet[0b11'000'111 | (t << 3)] =
		{
			"RST t",
			[this, t]() {
				bus->write(--SP, PC >> 8);
				bus->write(--SP, PC & 0x00FF);
				PC = t << 3;
			},
			4
		};
	}

	InstructionSet[0b00'100'111] =
	{
		"DAA",
		[this]() {
			switch (bus->read(PC - 2))
			{
			// ADD A, r
			case 0b10'000'000:
			case 0b10'000'001:
			case 0b10'000'010:
			case 0b10'000'011:
			case 0b10'000'100:
			case 0b10'000'101:
			case 0b10'000'111:
			// ADD A, n
			case 0b11'000'110:
			// ADD A, (HL)
			case 0b10'000'110:
			// ADC A, r
			case 0b10'001'000:
			case 0b10'001'001:
			case 0b10'001'010:
			case 0b10'001'011:
			case 0b10'001'100:
			case 0b10'001'101:
			case 0b10'001'111:
			// ADC A, (HL)
			case 0b10'001'110:
			// ADC A, nn
			case 0b11'001'110:
			{
				// ADD/ADC
				uint8_t AL = A & 0x0F, AH = A & 0xF0;
				if (CY == 0 && H == 0 && AL >= 0x0 && AL <= 0x9 && AH >= 0x0 && AH <= 0x9)
				{
					A += 0x00;
					CY = 0;
				}
				else if (CY == 0 && H == 0 && AL >= 0xA && AL <= 0xF && AH >= 0x0 && AH <= 0x8)
				{
					A += 0x06;
					CY = 0;
				}
				else if (CY == 0 && H == 1 && AL >= 0x0 && AL <= 0x3 && AH >= 0x0 && AH <= 0x9)
				{
					A += 0x06;
					CY = 0;
				}
				else if (CY == 0 && H == 0 && AL >= 0x0 && AL <= 0x9 && AH >= 0xA && AH <= 0xF)
				{
					A += 0x60;
					CY = 1;
				}
				else if (CY == 0 && H == 0 && AL >= 0xA && AL <= 0xF && AH >= 0x9 && AH <= 0xF)
				{
					A += 0x66;
					CY = 1;
				}
				else if (CY == 0 && H == 1 && AL >= 0x0 && AL <= 0x3 && AH >= 0xA && AH <= 0xF)
				{
					A += 0x66;
					CY = 1;
				}
				else if (CY == 1 && H == 0 && AL >= 0x0 && AL <= 0x9 && AH >= 0x0 && AH <= 0x2)
				{
					A += 0x60;
					CY = 1;
				}
				else if (CY == 1 && H == 0 && AL >= 0xA && AL <= 0xF && AH >= 0x0 && AH <= 0x2)
				{
					A += 0x66;
					CY = 1;
				}
				else if (CY == 1 && H == 1 && AL >= 0x0 && AL <= 0x3 && AH >= 0x0 && AH <= 0x3)
				{
					A += 0x66;
					CY = 1;
				}


				break;
			}

			// SUB r
			case 0b10'010'000:
			case 0b10'010'001:
			case 0b10'010'010:
			case 0b10'010'011:
			case 0b10'010'100:
			case 0b10'010'101:
			case 0b10'010'111:
			// SUB (HL)
			case 0b10'010'110:
			// SUB n
			case 0b11'010'110:
			// SBC A, r
			case 0b10'011'000:
			case 0b10'011'001:
			case 0b10'011'010:
			case 0b10'011'011:
			case 0b10'011'100:
			case 0b10'011'101:
			case 0b10'011'111:
			// SBC A. (HL)
			case 0b10'011'110:
			// SBC A, n
			case 0b11'011'110:
			{
				// SUB/SBC
				uint8_t AL = A & 0x0F, AH = A & 0xF0;
				if (CY ==0 && H ==0 && AL >= 0x0 && AL <= 0x9 && AH >= 0x0 && AH <= 0x9)
				{
					A += 0x00;
					CY = 0;
				}
				else if (CY ==0 && H ==1 && AL >= 0x6 && AL <= 0xF && AH >= 0x0 && AH <= 0x8)
				{
					A += 0xFA;
					CY = 0;
				}
				else if (CY == 1&& H == 0&& AL >= 0x0 && AL <= 0x9 && AH >= 0x7 && AH <= 0xF)
				{
					A += 0xA0;
					CY = 1;
				}
				else if (CY == 1&& H == 1&& AL >= 0x6 && AL <= 0xF && AH >= 0x6 && AH <= 0xF)
				{
					A += 0x9A;
					CY = 1;
				}
				break;
			}
			}

			HC = 0;
			Z = A == 0;
		},
		1
	};

	InstructionSet[0b00'101'111] =
	{
		"CPL (A <- ~A)",
		[this]() {
			A = ~A;
			H = 1;
			N = 1;
		},
		1
	};

	InstructionSet[0b00'111'111] =
	{
		"CCF (CY <- ~CY)",
		[this]() {
			CY = ~CY;
			H = 1;
			N = 1;
		},
		1
	};

	InstructionSet[0b00'110'111] =
	{
		"SCF (CY <- 1)",
		[this]() {
			CY = 1;
			H = 0;
			N = 0;
		},
		1
	};

	InstructionSet[0b11'110'011] =
	{
		"DI (IME <- 0)",
		[this]() {
			bus->IME = 0;
		},
		1
	};

	InstructionSet[0b11'110'011] =
	{
		"EI (IME <- 1)",
		[this]() {
			bus->IME = 1;
		},
		1
	};

	InstructionSet[0b01'110'110] =
	{
		"HALT",
		[this]() {
		// TODO
	},
	1
	};

	InstructionSet[0b00'010'000] =
	{
		"STOP",
		[this]() {
		// TODO
	},
	1
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

inline uint16_t& CPU::qq(uint8_t i)
{
	switch (i)
	{
	case 0b00:
		return BC;
	case 0b01:
		return DE;
	case 0b10:
		return HL;
	case 0b11:
		return AF;

	}
}

inline uint16_t& CPU::ss(uint8_t i)
{
	switch (i)
	{
	case 0b00:
		return BC;
	case 0b01:
		return DE;
	case 0b10:
		return HL;
	case 0b11:
		return SP;

	}
}

void CPU::reset()
{
	SP = 0xFFFE;
	//F = 0xB0;
	BC = 0X0013;
	DE = 0X00D8;
	HL = 0X014D;

	cycle = 0;
}
