#include <check.h>

#include <stdlib.h>

#include "util_tests.h"
#include "bits_and_bytes.h"

START_TEST (merge_bytes)
{
	uint8_t hi_byte[4] = {0xFF, 0x0F, 0x00, 0x20};
	uint8_t lo_byte[4] = {0xFF, 0x00, 0xE9, 0x17};
	uint16_t expected_result[4] = {0xFFFF
	                              , 0x0F00
	                              , 0x00E9
	                              , 0x2017};

	uint16_t word_result = append_hi_byte_to_lo_byte(hi_byte[_i], lo_byte[_i]);

	ck_assert_uint_eq(word_result, expected_result[_i]);
}

Suite* util_suite(void)
{
	Suite* s;
	TCase* tc_util;

	s = suite_create("Util Tests");
	tc_util = tcase_create("Util Functions");
	tcase_add_loop_test(tc_util, merge_bytes, 0, 4);
	suite_add_tcase(s, tc_util);

	return s;
}
