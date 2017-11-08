# cNES

A NES emulator written in C. Currently a work in progress.

## Current Status
- Implemented a rom loader with mapper 000 support
- Working CPU with an accurate nestest log up to line 5003
- See TODO section

## Current Branches
- cpu - testing read/write functions to work wit opcode functions.
- ppu - testing PPU implementation.

## Issues
- Error in my nestest log at line 5004 (start of illegal opcodes)
- No PPU or audio

## TODO
- Implement PPU
- Add more mappers

## Acknowledgments
- https://wwww.github.com/mwpenny/pureNES (followed their layout in cartridge.c for my cart.c)
- https://www.dwheeler.com/6502/oneelkruns/asm1step.html (very good information)
