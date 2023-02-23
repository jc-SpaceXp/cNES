#ifndef __CPU_TESTS__
#define __CPU_TESTS__

Suite* cpu_master_suite(void);
Suite* cpu_test_helpers_suite(void);
Suite* cpu_memory_access_suite(void);
Suite* cpu_bus_signals_suite(void);
Suite* cpu_address_mode_final_address_suite(void);
Suite* cpu_branch_address_suite(void);
Suite* cpu_single_cycle_suite(void);
Suite* cpu_isa_suite(void);
Suite* cpu_hardware_interrupts_suite(void);
Suite* cpu_trace_logger_suite(void);

#endif /* __CPU_TESTS__ */
