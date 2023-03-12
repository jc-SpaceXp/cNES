#include "bits_and_bytes.h"

uint16_t append_hi_byte_to_lo_byte(uint8_t hi_byte, uint8_t lo_byte)
{
	return (hi_byte << 8) | lo_byte;
}
