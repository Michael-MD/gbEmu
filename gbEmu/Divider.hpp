#pragma once
#include <cstdint>

template <typename T>
class Divider
{
public:
	Divider(T* ResetValue, T MaxValue)
	{
		Counter = *ResetValue;
		this->ResetValue = ResetValue;
		this->MaxValue = MaxValue;

		nOverflows = 0;
	}

	bool clock()
	{
		// If counter overflows then emit a 
		// signal.
		if (Counter++ == MaxValue)
		{
			Counter = *ResetValue;
			nOverflows++;
			return true;
		}

		return false;
	}
	
	// Keeps track of number of overflows
	uint64_t nOverflows;

private:
	// Keeps track of internal ticks
	T Counter;

	// The value reloaded into Counter on overflow
	T* ResetValue;

	// The value whcih counter cannot exceed
	T MaxValue;
};

