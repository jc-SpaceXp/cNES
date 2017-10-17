NEXT
> make get_op functoins return NES->RAM[operand] rather than operand, which then 
  gets passed to NES->RAM[] in the execute functions -- therefore also need to
  update execute functions to reflect those changes
>> get_op functions can now use the general read functions
> Increase RAM size to see if the read/write functions are ok
> potentially add cycles to cpu_struct - believe its needed later for accurate CPU emulation
> see tmp folder for updated: functions.h, functions_generic.h, execute.h and memory.h
> tmp folder is currently a work in progress

WORKING FUNCTIONS
>> Gets_op function fetched correct target address - then all that is needed is to read the addres - NESCPU->RAM[operand]
>> All get_op functions work
>> Also all flag (set and update) functions work
>> ADC works fine -- including the overflow - therefore can impliment SBC as ADC with the 2nd operand 2's complimented

DONE
> update execute.h to reflect changes in functions.h
>> condensed all functions
> update NES Program counter (for execute.c - included as a parameter now)

>>> Memory.h <<<<
- have option to either write to its address and each mirror (3 in total) or read from its absolute address (address & 0x07FF)
- have currently decided to read and write to an absolute address (non-mirrored section - as the mirrored paths are unimportant in a software context)


