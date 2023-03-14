#include <stdint.h>

unsigned get_nth_bit(unsigned input, unsigned bit_pos); // Only returns a 0 or 1
uint16_t append_hi_byte_to_lo_byte(uint8_t hi_byte, uint8_t lo_byte);
uint8_t get_lo_byte_from_addr(uint16_t addr);
uint8_t get_hi_byte_from_addr(uint16_t addr);
