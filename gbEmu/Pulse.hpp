#pragma once
#include "SoundChannel.hpp"

#include <cstdint>

class Pulse : public SoundChannel
{
public:
	Pulse();
	~Pulse();

	virtual uint8_t GetSample() override;
	void clock();

	// Sweep
	union NRx0Register
	{
		struct
		{
			uint8_t Step : 3;
			uint8_t Direction : 1;
			uint8_t Pace : 3;
			uint8_t _ : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx0;

	// Length Timer and Duty Cycle
	union NRx1Register
	{
		struct
		{
			uint8_t InitLenTimer : 6;
			uint8_t Duty : 2;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx1;

	// Volume and Envelope
	union NRx2Register
	{
		struct
		{
			uint8_t SweepPace : 3;
			uint8_t EnvDir : 1;
			uint8_t InitVol : 4;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx2;

	// Specifies low byte of period
	uint8_t* NRx3;

	// Period High and Control
	union NRx4Register
	{
		struct
		{
			uint8_t Period : 3;
			uint8_t _ : 3;
			uint8_t LenEnable : 1;
			uint8_t Trigger : 1;
		};

		uint8_t reg;

		void operator=(uint8_t reg_)
		{
			reg = reg_;
		};
	} *NRx4;

private:

};

