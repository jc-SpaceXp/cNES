>>> Functions.h <<<
> Has been re-worked:
>> includes new functions get_op()
>>> which is now beign dived into there respective address modes
   .. to elimate if/else-if chains to speed up fetching the operand

>> execute functions have much less code as a result
>> fixed INDX, INDY address modes as i was fetching the incorrect operand
>> about 50% through condensing all functions

NEXT
> update execute.h to reflect changes in functions.h
> update NES Program counter
> potentially add cycles to cpu_struct - believe its needed later for accurate CPU emulation


WORKING FUNCTIONS
>> Gets correct target address - then all that is needed is to read the addres - NESCPU->RAM[operand]
> get_op (INDX, INDY,  works so far) - all other functions where bassed off of get_op therefore get_op has same functionality as get_op_INDX etc.
> get_op_INDX
> get_op_INDY
> get_op_ABS_offset (tested ABSY, ABSX, ABS - for zero and non-zero values of offset) - returns little endian correctly
> get_op_ABS - returns little endian correctly
> get_op_IMM_ZP - returns correct IMM value, and correct ZP address 
