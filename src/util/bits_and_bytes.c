#include "bits_and_bytes.h"

// 0th indexed bit pos
unsigned get_nth_bit(unsigned input, unsigned bit_pos)
{
	return (input >> bit_pos) & 0x01;
}

uint16_t append_hi_byte_to_lo_byte(uint8_t hi_byte, uint8_t lo_byte)
{
	return (hi_byte << 8) | lo_byte;
}

uint8_t get_lo_byte_from_addr(uint16_t addr)
{
	return addr; // implicitly masked to uint8_t
}

uint8_t get_hi_byte_from_addr(uint16_t addr)
{
	return (addr >> 8);
}
