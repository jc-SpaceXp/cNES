#include <check.h>

#include "cpu_tests.h"
#include "ppu_tests.h"
#include "cpu_ppu_interface_tests.h"
#include "mappers_tests.h"

int main(void)
{
	int number_failed;
	Suite* s;
	SRunner* sr;

	// cpu tests
	sr = srunner_create(cpu_master_suite());
	srunner_add_suite(sr, cpu_test_helpers_suite());
	srunner_add_suite(sr, cpu_memory_access_suite());
	srunner_add_suite(sr, cpu_bus_signals_suite());
	srunner_add_suite(sr, cpu_address_mode_final_address_suite());
	srunner_add_suite(sr, cpu_branch_address_suite());
	srunner_add_suite(sr, cpu_single_cycle_suite());
	srunner_add_suite(sr, cpu_isa_suite());
	srunner_add_suite(sr, cpu_hardware_interrupts_suite());
	srunner_add_suite(sr, cpu_trace_logger_suite());

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

	// mapper tests
	sr = srunner_create(mapper_master_suite());
	srunner_add_suite(sr, mapper_000_suite());
	srunner_add_suite(sr, mapper_001_suite());

	srunner_run_all(sr, CK_NORMAL);
	number_failed += srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? 0 : 1;
}
