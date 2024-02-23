#pragma once
#include <cstdint>
#include "Pulse.hpp"
#include "SDL.h"

class GB;

class APU
{
public:
	GB* gb;
	Pulse pulse1;

	void connectGB(GB* gb);
	void clock();
	// Static so it can referenced as callback function
	static void AudioSample(void* userdata, Uint8* stream, int len);

	// Audio Master Control
	union NR52Register
	{
		struct
		{
			uint8_t bCH1 : 1;
			uint8_t bCH2 : 1;
			uint8_t bCH3 : 1;
			uint8_t bCH4 : 1;
			uint8_t _ : 3;
			uint8_t bAPU : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR52;

	// Sound Panning
	union NR51Register
	{
		struct
		{
			uint8_t CH1R : 1;
			uint8_t CH2R : 1;
			uint8_t CH3R : 1;
			uint8_t CH4R : 1;
			uint8_t CH1L : 1;
			uint8_t CH2L : 1;
			uint8_t CH3L : 1;
			uint8_t CH4L : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR51;

	// Master Volume and VIN Panning
	union NR50Register
	{
		struct
		{
			uint8_t VolR : 3;
			uint8_t VINR : 1;
			uint8_t VolL : 3;
			uint8_t VINL : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NR50;


};

