# cNES

A NES emulator written in C. Currently a work in progress. Wanted to make this project ......

## Current Status
- Implemented a rom loader with mapper 000 support
- Working CPU with an 100% accurate nestest log up to line ~804
- See TODO section

## Issues
- Error in my nestest log at line ~804
- No PPU or audio

## TODO
- Implement PPU
- Add more mappers
- Fix issues discoreved by nestest.nes

## Acknowledgments
- https://wwww.github.com/mwpenny/pureNES (followed their layout in cartridge.c for my cart.c)
- https://www.dwheeler.com/6502/oneelkruns/asm1step.html (very good information)
