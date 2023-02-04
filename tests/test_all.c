#include <check.h>

#include "cpu_tests.h"
#include "ppu_tests.h"
#include "cpu_ppu_interface_tests.h"

int main(void)
{
	int number_failed;
	Suite* s;
	SRunner* sr;

	s = cpu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	// ppu tests
	s = ppu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed += srunner_ntests_failed(sr);
	srunner_free(sr);

	// cpu ppu tests
	s = cpu_ppu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed += srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? 0 : 1;
}
