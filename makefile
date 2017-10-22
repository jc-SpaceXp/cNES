CC = cc
CFLAGS = -Wall -std=c99

all: emu debug_emu

cpu.o: cpu.c cpu.h
	$(CC) $(CFLAGS) -c cpu.c

functions_generic.o: functions_generic.c functions_generic.h
	$(CC) $(CFLAGS) -c functions_generic.c

functions.o: functions.c functions.h functions_generic.h
	$(CC) $(CFLAGS) -c functions.c

opcode_execute.o: opcode_execute.c opcode_execute.h functions_generic.h functions.h
	$(CC) $(CFLAGS) -c opcode_execute.c

emu.o: emu.c cpu.h opcode_execute.h
	$(CC) $(CFLAGS) -c emu.c

emu: emu.o cpu.o functions_generic.o functions.o opcode_execute.o
	$(CC) -o emu emu.o cpu.o functions_generic.o functions.o opcode_execute.o

functions_debug.o: functions_debug.c functions.h functions_generic.h
	$(CC) $(CFLAGS) -c functions_debug.c

opcode_debug.o: opcode_debug.c opcode_debug.h functions_generic.h functions.h
	$(CC) $(CFLAGS) -c opcode_debug.c

debug_emu.o: debug_emu.c cpu.h opcode_debug.h
	$(CC) $(CFLAGS) -c debug_emu.c

debug_emu: debug_emu.o cpu.o functions_generic.o functions_debug.o opcode_debug.o
	$(CC) -o debug_emu debug_emu.o cpu.o functions_generic.o functions_debug.o opcode_debug.o

clean:
	rm *.o
