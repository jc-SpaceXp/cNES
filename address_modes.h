/*************************************************************
* Don't think i need this file - added into cpu_struct.h 
* (can move back here if cpu_struct needs de-cluttering
**************************************************************/

#ifndef __6502_ENUM__
#define __6502_ENUM__


/* Includes addressing modes
 * Adressing Modes:
 *
 * 1. ABS = Absolute Mode
 * 2. ABSX = Absolute Mode indexed via X
 * 3. ABSY = Absolute Mode indexed via Y
 * 4. ACC = Accumulator Mode
 * 5. IMM = Immediate Mode
 * 6. IMP = Implied Mode
 * 7. IND = Indirect Mode
 * 8. INDX = Indexed Indirect Mode via X
 * 9. INDY = Indirect Index Mode via Y
 * 10. REL = Relative Mode
 * 11. ZP = Zero Page Mode
 * 12. ZPX = Zero Page Mode indexed via X
 * 13. ZPY = Zero Page Mode indexed via Y
 */


/* Don't think that is needed */
enum MODE {
	ABS,
	ABSX,
	ABSY,
	ACC,
	IMM,
	IMP,
	IND,
	INDX,
	INDY,
	REL,
	ZP,
	ZPX,
	ZPY,
} address_modes;

#endif
