#include <check.h>

START_TEST (dummy_test)
{
	ck_assert_int_eq(5, 5);
}
END_TEST

Suite * dummy_suite(void)
{
	Suite* s;
	TCase* tc_core;

	s = suite_create("Dummy Tests");
	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, dummy_test);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite* s;
	SRunner* sr;

	s = dummy_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? 0 : 1;
}
