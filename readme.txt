>>> Functions.h <<<
> Has been re-worked:
>> includes new functions get_op() (refactored into seperate address modes)
>>> which is now beign dived into there respective address modes
   .. to elimate if/else-if chains to speed up fetching the operand

>> execute functions have much less code as a result
>> fixed INDX, INDY address modes as i was fetching the incorrect operand
>> about 50% through condensing all functions (done)

NEXT
> update execute.h to reflect changes in functions.h (done)
> update NES Program counter (done)
> potentially add cycles to cpu_struct - believe its needed later for accurate CPU emulation
> make get_op functoins return NES->RAM[operand] rather than operand, which then 
  gets passed to NES->RAM[] in the execute functions -- therefore also need to
  update execute functions to reflect those changes
> Increase RAM size to see if the read/write functions are ok


WORKING FUNCTIONS
>> Gets_op function fetched correct target address - then all that is needed is to read the addres - NESCPU->RAM[operand]
>> All get_op functions work
>> Also all flag (set and update) functions work
