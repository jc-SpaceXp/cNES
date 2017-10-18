CC = cc
CFLAGS = -Wall -std=c99

all: emu

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
	$(cc) -o emu emu.o cpu.o functions_generic.o functions.o opcode_execute.o

clean:
	rm *.o
