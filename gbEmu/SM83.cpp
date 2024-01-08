#include "SM83.hpp"
#include "GB.hpp"

#define DEBUG_MODE 1

#if DEBUG_MODE
	#include <iostream>
#endif
#include <sstream>

void SM83::connectGB(GB* gb)
{
	this->gb = gb;
}

void SM83::clock()
{
	if (gb->nClockCycles % 4 == 0) // CPU instructuctions defined in machine cycles
	{
		// =============== Intsruction Execution =============== 
		if (cycle == 0)
		{
			// Check for interrupts between intruction fetch cycles
			if (gb->IME)
			{
				// Check if any interrupts have occured
				if ((gb->IE->reg & gb->IF->reg & 0x1F) != 0)
				{
					// Check interrupts in order of precedence

					gb->IME = 0;
					gb->write(--SP, PC >> 8);
					gb->write(--SP, PC & 0x00FF);

					if (gb->IE->VerticalBlanking && gb->IF->VerticalBlanking)
					{
						PC = 0x0040;
					}
					else if (gb->IE->LCDC && gb->IF->LCDC)
					{
						PC = 0x0048;
					}
					else if (gb->IE->TimerOverflow && gb->IF->TimerOverflow)
					{
						PC = 0x0050;
					}
					else if (gb->IE->SerialIOTransferCompletion && gb->IF->SerialIOTransferCompletion)
					{
						PC = 0x0058;
					}
					else if (gb->IE->PNegEdge && gb->IF->PNegEdge)
					{
						PC = 0x0060;
					}
				}
			}

			// Fetch Next Instruction
			uint8_t data = gb->read(PC++);
			CurrentInstruction = InstructionSet[data];
			cycle += CurrentInstruction.cycles;
			a = CurrentInstruction.a;
			b = CurrentInstruction.b;

			#if DEBUG_MODE
				if (gb->nClockCycles % 100'000 == 0)
				{
					std::cout << std::hex << (int)(PC - 1) << std::dec << ' ' << CurrentInstruction.mnemonic() << ' ' << std::hex << (int)(data) << std::dec << ' ' << (int)Z;
					std::cout << std::hex << ' ' << (int)B << std::endl;
				
					std::cout << "Debug Message: " << gb->SerialOut << std::endl;
				}
			#endif

			CurrentInstruction.op();
		}
		else
		{
			// Continue Current Instruction Execution
			cycle--;
		}
	}

	// IME reset after interrupt occurs	
}


SM83::SM83()
{

	// TODO: Internal Checks
 
	InstructionSet[0b00'000'000] =
	{
		[]() {
			return "NOP ()";
		},
		[this]() {

		},
		1
	};
	
	for (uint8_t i = 0; i <= 7; i+1 == 0b110 ? i += 2 : i++)
	{
		for (uint8_t j = 0; j <= 7; j + 1 == 0b110 ? j += 2 : j++)
		{
			InstructionSet[0x40|(i<<3)|j] =
			{
				[]() {
					return "LD r,r' (r <- r')"
	;			},
				[this]() {
					uint8_t& r = GPR(a), &rp = GPR(b);
					r = rp;
				},
				1,
				i,
				j
			};
		}
	}

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0x00 | (i << 3) | 0b110] =
		{
			[this]() {
				uint8_t r = GPR(a);
				uint8_t rStr = GPRString(a);
				std::stringstream s;
				s << "LD " << rStr << ", [$" << std::hex << (int)PC << std::dec << "] = $" << std::hex << (int)gb->read(PC) << std::dec << " (r <- n)";
				return s.str();
;			},
			[this]() {
				uint8_t& r = GPR(a);
				r = gb->read(PC++);
			},
			2,
			i
		};

		InstructionSet[(0b01 << 6) | (i << 3) | 0b110] =
		{
			[]() {
				return "LD r, (HL) (r <- (HL))"
;			},
			[this]() {
				uint8_t& r = GPR(a);
				r = gb->read(HL);
			},
			2,
			i
		};
	}

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		
		InstructionSet[0b01'110'000 | i] =
		{
			[]() {
				return "LD (HL),r ((HL) <- r)"
;			},
			[this]() {
				uint8_t &r = GPR(a);
				gb->write(HL, r);
			},
			2,
			i
		};
	}

	InstructionSet[0b00'110'110] =
	{
		[]() {
			return "LD (HL), n ((HL) <- n)";
		},
		[this]() {
			gb->write(HL, gb->read(PC++));
		},
		3
	};

	InstructionSet[0b00'001'010] =
	{
		[]() {
			return "LD A, (BC) (A <- (BC))";
		},
		[this]() {
			A = gb->read(BC);
		},
		2
	};

	InstructionSet[0b00'011'010] =
	{
		[]() {
			return "LD A, (DE)  (A <- (DE))";
		},
		[this]() {
			A = gb->read(DE);
		},
		2
	};

	InstructionSet[0b11'110'010] =
	{
		[]() {
			return "LD A, (C) (A <- (0xFF00 + C))";
		},
		[this]() {
			A = gb->read(0xFF00 + C);
		},
		2
	};

	InstructionSet[0b11'100'010] =
	{
		[]() {
			return "LD (C), A ((0xFF00H+C) <- A)";
		},
		[this]() {
			gb->write(0xFF00 + C, A);
		},
		2
	};

	InstructionSet[0b11'110'000] =
	{
		[this]() {
			std::stringstream s;
			s << "LD A, [$" << std::hex << (int)(0xFF00 + gb->read(PC)) << std::dec << "] = $" << std::hex << (int)gb->read(0xFF00 + gb->read(PC)) << std::dec << " (A <-(n))";
			return s.str();
		},
		[this]() {
			A = gb->read(0xFF00 + gb->read(PC++));
		},
		3
	};

	InstructionSet[0b11'100'000] =
	{
		[]() {
			return "LD (n), A ((n) <- A)";
		},
		[this]() {
			gb->write(0xFF00 + gb->read(PC++), A);
		},
		3
	};

	InstructionSet[0b11'111'010] =
	{
		[]() {
			return "LD A, (nn)(A <- (nn))";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			A = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'101'010] =
	{
		[]() {
			return "LD (nn), A ((nn) <- A)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			gb->write((HI << 8) | LO, A);
		},
		4
	};

	InstructionSet[0b00'101'010] =
	{
		[]() {
			return "LD A, (HLI) (A <- (HL), HL <- HL + 1)";
		},
		[this]() {
			A = gb->read(HL++);
		},
		2
	};

	InstructionSet[0b00'111'010] =
	{
		[]() {
			return "LD A, (HLD) (A <- (HL), HL <- HL1)";
		},
		[this]() {
			A = gb->read(HL--);
		},
		2
	};

	InstructionSet[0b00'000'010] =
	{
		[]() {
			return "LD (BC), A ((BC) <- A)";
		},
		[this]() {
			gb->write(BC, A);
		},
		2
	};

	InstructionSet[0b00'010'010] =
	{
		[]() {
			return "LD (DE), A ((DE) <- A)";
		},
		[this]() {
			gb->write(DE, A);
		},
		2
	};

	InstructionSet[0b00'100'010] =
	{
		[]() {
			return "LD (HLI), A ((HL) <- A HL <- HL + 1)";
		},
		[this]() {
			gb->write(HL++, A);
		},
		2
	};

	InstructionSet[0b00'110'010] =
	{
		[this]() {
			std::stringstream s;
			s << "LD [HL-] = $" << std::hex << (int)HL << std::dec << ", A = $" << std::hex << (int)A << std::dec << " ((HL) <- A, HL <- HL-1)";
			return s.str();
		},
		[this]() {
			gb->write(HL--, A);
		},
		2
	};

	InstructionSet[0b00'000'001] =
	{
		[]() {
			return "LD dd, nn (BC <- nn)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			BC = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'010'001] =
	{
		[]() {
			return "LD dd, nn (DE <- nn)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			DE = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'100'001] =
	{
		[]() {
			return "LD dd, nn (HL <- nn)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			HL = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b00'110'001] =
	{
		[]() {
			return "LD dd, nn (SP <- nn)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			SP = (HI << 8) | LO;
		},
		3
	};

	InstructionSet[0b11'111'001] =
	{
		[]() {
			return "LD SP 11 SP, HL (SP <- HL)";
		},
		[this]() {
			SP = HL;
		},
		2
	};

	for (uint8_t i = 0; i <= 3; i++)
	{
		

		InstructionSet[0b11'000'101 | (i << 4)] =
		{
			[]() {
				return "PUSH qq ((SP-1) <- qqH (SP - 2) <- qqL SP <- SP - 2)"
;			},
			[this]() {
				uint16_t& r = qq(a);
				gb->write(--SP, r >> 8);
				gb->write(--SP, r & 0x0F);
			},
			4,
			i
		};

		InstructionSet[0b11'000'001 | (i << 4)] =
		{
			[]() {
				return "POP qq (qqL <- (SP) qqH <- (SP + 1) SP <- SP + 2)"
;			},
			[this]() {
				uint16_t& r = qq(a);

				r = (r & 0xFF00) | gb->read(SP++);
				r = (gb->read(SP++) << 8) | (r & 0x00FF);
			},
			3,
			i
		};
	}

	InstructionSet[0b11'111'000] =
	{
		[]() {
			return "LDHL SP, e (HL <- SP+e)";
		},
		[this]() {
			int8_t e = gb->read(PC++);
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
		[]() {
			return "LD (nn), SP ((nn) <- SPL (nn + 1) <- SPH)";
		},
		[this]() {
			uint8_t LO, HI;
			LO = gb->read(PC++);
			HI = gb->read(PC++);
			uint16_t M = (HI << 8) | LO;
			gb->write(M++, SP & 0x00FF);
			gb->write(M, SP & 0xFF00);
		},
		5
	};

	InstructionSet[0b11'000'110] =
	{
		[]() {
			return "ADD A,n (A <- A+n)";
		},
		[this]() {
			uint8_t n = gb->read(PC++);
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
		[]() {
			return "ADD A, (HL) (A <- A+(HL))";
		},
		[this]() {
			A += gb->read(HL);
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		
		InstructionSet[0b10'001'000 | i] =
		{
			[]() {
				return "ADC A, r (A <- A+s+CY)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				uint16_t tmp = A + r + CY;
				uint8_t tmp2 = ((A & 0xF) + (r & 0xF) + CY) >> 4;

				HC = tmp2 != 0;
				CY = (tmp >> 8) != 0;
				A = tmp;
				Z = A == 0;
				N = 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'001'110] =
	{
		[]() {
		return 	" ADC A, n (A <- A+n+CY)";
		},
		[this]() {
			uint8_t n = gb->read(PC++);
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
		[]() {
		return 	" ADC A, (HL) (A <- A+n+CY)";
		},
		[this]() {
			uint8_t M = gb->read(HL);
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

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b10'010'000 | i] =
		{
			[]() {
				return "SUB r (A <- A-r)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				HC = (A & 0xF) < (r & 0xF);
				CY = A < r;

				A -= r;
				Z = A == 0;
				N = 1;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'010'110] =
	{
		[]() {
			return "SUB n ( A <- A-n)";
		},
		[this]() {
			uint8_t n = gb->read(PC++);

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
		[]() {
			return "SUB (HL) ( A <- A-(HL))";
		},
		[this]() {
			uint8_t M = gb->read(HL);

			HC = (A & 0xF) < (M & 0xF);
			CY = A < M;

			A -= M;
			Z = A == 0;
			N = 1;
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		
		InstructionSet[0b10'011'000 | i] =
		{
			[]() {
				return "SBC A, r (A <- A-r-CY)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				HC = (A & 0xF) < ((r + 1) & 0xF);
				CY = A < (r + 1);

				A -= (r + 1);
				Z = A == 0;
				N = 1;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'011'110] =
	{
		[]() {
			return "SBC A, n (A <- A - n - CY)";
		},
		[this]() {
			uint8_t n = gb->read(PC++);

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
		[]() {
			return "SBC A, (HL) (A <- A - (HL) - CY)";
		},
		[this]() {
			uint8_t M = gb->read(HL);

			HC = (A & 0xF) < ((M + 1) & 0xF);
			CY = A < (M + 1);

			A -= (M + 1);
			Z = A == 0;
			N = 1;
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b10'100'000 | i] =
		{
			[]() {
				return "AND r (A & r)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				A &= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'100'110] =
	{
		[]() {
			return "AND n (A & n)";
		},
		[this]() {
			A &= gb->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'100'110] =
	{
		[]() {
			return "AND (HL) (A & (HL))";
		},
		[this]() {
			A &= gb->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b10'110'000 | i] =
		{
			[]() {
				return "OR r (A | r)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				A |= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'110'110] =
	{
		[]() {
			return "OR n (A | n)";
		},
		[this]() {
			A |= gb->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'110'110] =
	{
		[]() {
			return "OR (HL) (A | (HL))";
		},
		[this]() {
			A |= gb->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b10'101'000 | i] =
		{
			[]() {
				return "XOR r (A ^ r)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				A ^= r;
				CY = 0;
				HC = 0;
				N = 0;
				Z = A == 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'101'110] =
	{
		[]() {
			return "XOR n (A ^ n)";
		},
		[this]() {
			A ^= gb->read(PC++);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	InstructionSet[0b10'101'110] =
	{
		[]() {
			return "XOR (HL) (A ^ (HL))";
		},
		[this]() {
			A ^= gb->read(HL);
			CY = 0;
			HC = 0;
			N = 0;
			Z = A == 0;
		},
		2
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b10'111'000 | i] =
		{
			[]() {
				return "CP r (A == r)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				Z = A == r;
				HC = (A & 0xF) < (r & 0xF);
				N = 1;
				CY = A < r;
			},
			1,
			i
		};
	}

	InstructionSet[0b11'111'110] =
	{
		[this]() {
			std::stringstream s;
			s << "CP $" << std::hex << (int)gb->read(PC) << std::dec << " (A == n)";
			return s.str();
		},
		[this]() {
			uint8_t n = gb->read(PC++);
			Z = A == n;
			HC = (A & 0xF) < (n & 0xF);
			N = 1;
			CY = A < n;
		},
		1
	};

	InstructionSet[0b10'111'110] =
	{
		[]() {
			return "CP (HL) (A == (HL))";
		},
		[this]() {
			uint8_t M = gb->read(HL);
			Z = A == M;
			HC = (A & 0xF) < (M & 0xF);
			N = 1;
			CY = A < M;
		},
		1
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b00'000'100 | (i << 3)] =
		{
			[]() {
				return "INC r (r <- r+1)"
;			},
			[this]() {
				uint8_t& r = GPR(a);

				HC = ((r & 0xF) + (1 & 0xF)) >> 4;
				N = 0;
				r += 1;
				Z = r == 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b00'110'100] =
	{
		[]() {
			return "INC (HL) ((HL) <- (HL)+1)";
		},
		[this]() {
			uint8_t M = gb->read(HL);
			HC = ((M & 0xF) + (1 & 0xF)) >> 4;
			N = 0;
			M += 1;
			Z = M == 0;
			gb->write(HL, M);
		},
		3
	};

	for (uint8_t i = 0; i <= 7; i + 1 == 0b110 ? i += 2 : i++)
	{
		InstructionSet[0b00'000'101 | (i << 3)] =
		{
			[this]() {
				std::stringstream s;
				uint8_t r = GPR(a);
				char rStr = GPRString(a);
				s << "DEC " << rStr << " = $" << std::hex << (int)r << std::dec << " (r <- r-1)";
				return s.str();
;			},
			[this]() {
				uint8_t& r = GPR(a);
				HC = (r & 0xF) < (1 & 0xF);
				N = 1;
				r--;
				Z = r == 0;
			},
			1,
			i
		};
	}

	InstructionSet[0b00'110'101] =
	{
		[]() {
			return "DEC (HL) ((HL) <- (HL)-1)";
		},
		[this]() {
			uint8_t M = gb->read(HL);
			HC = (M & 0xF) < (1 & 0xF);
			N = 1;
			M -= 1;
			Z = M == 0;
			gb->write(HL, M);
		},
		3
	};

	InstructionSet[0b00'001'001] =
	{
		[]() {
			return "ADD HL,BC (HL <- HL+BC)";
		},
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
		[]() {
			return "ADD HL,DE (HL <- HL+DE)";
		},
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
		[]() {
			return "ADD HL,HL (HL <- HL+HL)";
		},
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
		[]() {
			return "ADD HL,SP (HL <- HL+SP)";
		},
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
		[]() {
			return "ADD SP,e (SP <- SP+e)";
		},
		[this]() {
			int8_t e = gb->read(PC++);
			HC = (((SP & 0xFFF) + (e & 0xFFF)) >> 12) != 0;
			CY = (((uint32_t)SP + (uint32_t)e) >> 16) != 0;
			SP += e;
			N = 0;
			Z = 0;
		},
		4
	};
	
	for (uint8_t i = 0; i <= 3; i++)
	{
		InstructionSet[0b00'000'011 | (i << 4)] =
		{
			[]() {
				return "INC ss (ss <- ss + 1)"
;			},
			[this]() {
				uint16_t& r = ss(a);

				r += 1;
			},
			2,
			i
		};

		InstructionSet[0b00'001'011 | (i << 4)] =
		{
			[]() {
				return "DEC ss (ss <- ss - 1)"
;			},
			[this]() {
				uint16_t& r = ss(a);

				r -= 1;
			},
			2,
			i
		};
	}

	InstructionSet[0b00'000'111] =
	{
		[]() {
			return "RLCA";
		},
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
		[]() {
			return "RLA";
		},
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
		[]() {
			return "RRCA";
		},
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
		[]() {
			return "RRA";
		},
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
		[]() {
			return "Shift";
		},
		[this]() {
			uint8_t D = gb->read(PC++);
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
					uint8_t M = gb->read(HL);
					CY = M >> 7;
					M <<= 1;
					M |= CY;
					gb->write(HL, M);
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
					uint8_t M = gb->read(HL);
					uint8_t tmp = M >> 7;
					M <<= 1;
					M |= CY;
					gb->write(HL, M);
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
					uint8_t M = gb->read(HL);
					CY = M & 0x01;
					M >>= 1;
					M |= CY << 7;
					gb->write(HL, M);
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
					uint8_t M = gb->read(HL);
					uint8_t tmp = M & 0x01;
					M >>= 1;
					M |= CY << 7;
					CY = tmp;
					gb->write(HL, M);
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
					uint8_t M = gb->read(HL);
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
					uint8_t M = gb->read(HL);
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
					uint8_t M = gb->read(HL);
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
					uint8_t M = gb->read(HL);
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
		[]() {
			return "BIT";
		},
		[this]() {
			uint8_t D = gb->read(PC++);
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
					uint8_t M = gb->read(HL);
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
					uint8_t M = gb->read(HL);
					M |= (1 << b);
					gb->write(HL, M);

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
					uint8_t M = gb->read(HL);
					M &= ~(1 << b);
					gb->write(HL, M);

					cycle += 2;
				}
				break;
			}
		},
		2
	};
	
	InstructionSet[0b11'000'011] =
	{
		[]() {
			return "JP nn (PC <- nn)";
		},
		[this]() {
			uint8_t LO = gb->read(PC++);
			uint8_t HI = gb->read(PC++);
			PC = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'000'010] =
	{
		[]() {
			return "JP ~Z, nn (If ~Z: PC <- nn)";
		},
		[this]() {
			if (Z == 0)
			{
				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
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
		[]() {
			return "JP Z, nn (If Z: PC <- nn)";
		},
		[this]() {
			if (Z == 1)
			{
				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
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
		[]() {
			return "JP ~CY, nn (If ~CY: PC <- nn)";
		},
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
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
		[]() {
			return "JP CY, nn (If CY: PC <- nn)";
		},
		[this]() {
			if (CY == 1)
			{
				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
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
		[]() {
			return "JR e (PC <- PC+e)";
		},
		[this]() {
			int8_t e = gb->read(PC++) - 1;
			PC += e;
		},
		3
	};

	InstructionSet[0b00'100'000] =
	{
		[]() {
			return "JR ~Z, e (If ~Z: PC <- PC+e)";
		},
		[this]() {
			if (Z == 0)
			{
				int8_t e = gb->read(PC++) - 1;
				PC += e;
				cycle++;
			}

			PC++;
		},
		2
	};

	InstructionSet[0b00'101'000] =
	{
		[]() {
			return "JR Z, e (If Z: PC <- PC+e)";
		},
		[this]() {
			if (Z == 1)
			{
				int8_t e = gb->read(PC++) - 1;
				PC += e;
				cycle++;
			}

			PC++;
		},
		2
	};

	InstructionSet[0b00'110'000] =
	{
		[]() {
			return "JR ~CY, e (If ~CY: PC <- PC+e)";
		},
		[this]() {
			if (CY == 0)
			{
				int8_t e = gb->read(PC++) - 1;
				PC += e;
				cycle++;
			}

			PC++;
		},
		2
	};

	InstructionSet[0b00'111'000] =
	{
		[]() {
			return "JR CY, e (If CY: PC <- PC+e)";
		},
		[this]() {
			if (CY == 1)
			{
				int8_t e = gb->read(PC++) - 1;
				PC += e;
				cycle++;
			}

			PC++;
		},
		2
	};

	InstructionSet[0b11'101'001] =
	{
		[]() {
			return "JP (HL) (PC <- HL)";
		},
		[this]() {
			PC = HL;
		},
		1
	};

	InstructionSet[0b11'001'101] =
	{
		[]() {
			return "CALL nn";
		},
		[this]() {
			gb->write(--SP, PC >> 8);
			gb->write(--SP, PC & 0x00FF);

			uint8_t LO = gb->read(PC++);
			uint8_t HI = gb->read(PC++);
			PC = (HI << 8) | LO;
		},
		6
	};

	InstructionSet[0b11'000'100] =
	{
		[]() {
			return "CALL cc, ~Z";
		},
		[this]() {
			if (Z == 0)
			{
				gb->write(--SP, PC >> 8);
				gb->write(--SP, PC & 0x00FF);

				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'001'100] =
	{
		[]() {
			return "CALL cc, Z";
		},
		[this]() {
			if (Z == 1)
			{
				gb->write(--SP, PC >> 8);
				gb->write(--SP, PC & 0x00FF);

				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'010'100] =
	{
		[]() {
			return "CALL cc, ~CY";
		},
		[this]() {
			if (CY == 0)
			{
				gb->write(--SP, PC >> 8);
				gb->write(--SP, PC & 0x00FF);

				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'011'100] =
	{
		[]() {
			return "CALL cc, CY";
		},
		[this]() {
			if (CY == 1)
			{
				gb->write(--SP, PC >> 8);
				gb->write(--SP, PC & 0x00FF);

				uint8_t LO = gb->read(PC++);
				uint8_t HI = gb->read(PC++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		3
	};

	InstructionSet[0b11'001'001] =
	{
		[]() {
			return "RET";
		},
		[this]() {
			uint8_t LO = gb->read(SP++);
			uint8_t HI = gb->read(SP++);
			PC = (HI << 8) | LO;
		},
		4
	};

	InstructionSet[0b11'011'001] =
	{
		[]() {
			return "RETI";
		},
		[this]() {
			uint8_t LO = gb->read(SP++);
			uint8_t HI = gb->read(SP++);
			PC = (HI << 8) | LO;
			gb->IME = 1; // TODO: Double Check
		},
		4
	};

	InstructionSet[0b11'000'000] =
	{
		[]() {
			return "RET ~Z";
		},
		[this]() {
			if (Z == 0)
			{
				uint8_t LO = gb->read(SP++);
				uint8_t HI = gb->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'001'000] =
	{
		[]() {
			return "RET Z";
		},
		[this]() {
			if (Z == 1)
			{
				uint8_t LO = gb->read(SP++);
				uint8_t HI = gb->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'010'000] =
	{
		[]() {
			return "RET ~CY";
		},
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = gb->read(SP++);
				uint8_t HI = gb->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	InstructionSet[0b11'011'000] =
	{
		[]() {
			return "RET CY";
		},
		[this]() {
			if (CY == 0)
			{
				uint8_t LO = gb->read(SP++);
				uint8_t HI = gb->read(SP++);
				PC = (HI << 8) | LO;

				cycle += 3;
			}
		},
		2
	};

	for(uint8_t t = 0; t <= 7; t++)
	{
		InstructionSet[0b11'000'111 | (t << 3)] =
		{
			[]() {
				return "RST t"
;			},
			[this, t]() {
				gb->write(--SP, PC >> 8);
				gb->write(--SP, PC & 0x00FF);
				PC = t << 3;
			},
			4,
			t
		};
	}

	InstructionSet[0b00'100'111] =
	{
		[]() {
			return "DAA";
		},
		[this]() {
			switch (gb->read(PC - 2))
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
		[]() {
			return "CPL (A <- ~A)";
		},
		[this]() {
			A = ~A;
			H = 1;
			N = 1;
		},
		1
	};

	InstructionSet[0b00'111'111] =
	{
		[]() {
			return "CCF (CY <- ~CY)";
		},
		[this]() {
			CY = ~CY;
			H = 1;
			N = 1;
		},
		1
	};

	InstructionSet[0b00'110'111] =
	{
		[]() {
			return "SCF (CY <- 1)";
		},
		[this]() {
			CY = 1;
			H = 0;
			N = 0;
		},
		1
	};

	InstructionSet[0b11'110'011] =
	{
		[]() {
			return "DI (IME <- 0)";
		},
		[this]() {
			gb->IME = 0;
		},
		1
	};

	InstructionSet[0b11'111'011] =
	{
		[]() {
			return "EI (IME <- 1)";
		},
		[this]() {
			gb->IME = 1;
		},
		1
	};

	InstructionSet[0b01'110'110] =
	{
		[]() {
			return "HALT";
		},
		[this]() {
		// TODO
	},
	1
	};

	InstructionSet[0b00'010'000] =
	{
		[]() {
			return "STOP";
		},
		[this]() {
		// TODO
	},
	1
	};
}

inline uint8_t& SM83::GPR(uint8_t i)
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

inline char SM83::GPRString(uint8_t i)
{
	switch (i)
	{
	case 0b111:
		return 'A';
	case 0b000:
		return 'B';
	case 0b001:
		return 'C';
	case 0b010:
		return 'D';
	case 0b011:
		return 'E';
	case 0b100:
		return 'H';
	case 0b101:
		return 'L';
	}
}

inline uint16_t& SM83::qq(uint8_t i)
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

inline uint16_t& SM83::ss(uint8_t i)
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

void SM83::reset()
{
	SP = 0xFFFE;
	//F = 0xB0;
	BC = 0X0013;
	DE = 0X00D8;
	HL = 0X014D;

	cycle = 0;
}
