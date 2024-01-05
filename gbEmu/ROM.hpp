#include <cstdint>

class ROM : public 
{
public:
	void cartWrite(uint16_t addr, uint8_t data) override;
	uint8_t cartRead(uint8_t data) override;
};