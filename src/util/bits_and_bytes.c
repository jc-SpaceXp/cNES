#include "bits_and_bytes.h"

// 0th indexed bit position
unsigned get_nth_bit(unsigned input, unsigned bit_pos)
{
	return (input >> bit_pos) & 0x01;
}
