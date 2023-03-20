#include "bits_and_bytes.h"

// 0th indexed bit position
unsigned get_nth_bit(unsigned input, unsigned bit_pos)
{
	return (input >> bit_pos) & 0x01;
}

uint16_t append_hi_byte_to_lo_byte(uint8_t hi_byte, uint8_t lo_byte)
{
	return (hi_byte << 8) | lo_byte;
}
