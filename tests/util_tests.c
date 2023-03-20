#include <check.h>

#include <stdlib.h>

#include "util_tests.h"
#include "bits_and_bytes.h"

START_TEST (check_bit_pos_is_set)
{
	uint16_t bit_check = 0xC02F;
	// 0th to 3rd bit, 4th to 7th, etc.
	unsigned expected_result[16] = { 1, 1, 1, 1
	                               , 0, 1, 0, 0
	                               , 0, 0, 0, 0
	                               , 0, 0, 1, 1 };

	unsigned return_val = get_nth_bit(bit_check, _i);

	ck_assert_uint_le(return_val, 2); // result should either be 0 or 1
	ck_assert_uint_eq(return_val, expected_result[_i]);
}

START_TEST (merging_two_byte)
{
	uint8_t hi_byte[4] = {0xFF, 0x0F, 0x00, 0x20};
	uint8_t lo_byte[4] = {0xFF, 0x00, 0xE9, 0x17};
	uint16_t expected_result[4] = { 0xFFFF
	                              , 0x0F00
	                              , 0x00E9
	                              , 0x2017 };

	unsigned merge_result = append_hi_byte_to_lo_byte(hi_byte[_i], lo_byte[_i]);

	ck_assert_uint_eq(merge_result, expected_result[_i]);
}


Suite* util_suite(void)
{
	Suite* s;
	TCase* tc_util;

	s = suite_create("Util Tests");
	tc_util = tcase_create("Util Functions");
	tcase_add_loop_test(tc_util, check_bit_pos_is_set, 0, 16);
	tcase_add_loop_test(tc_util, merging_two_byte, 0, 4);
	suite_add_tcase(s, tc_util);

	return s;
}
