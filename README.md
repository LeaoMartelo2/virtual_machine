# virtual\_machine

Simple attempt at implementing a virtual machine and a bytecode language that runs on it.

In the future i will be implementing a way to assemble human readable code in to the machine's bytecode.


### Opcodes implemented

| Opcode |  INFO | Argument count | Example |
| -------| ------|----------------|---------|
| NO\_OP |  No operation. | 0 | no\_op|
| HALT   |  Halts the machine. | 0 | halt |
| STATE\_DUMP | Prints the value of the registers, program size and program counter | 0 | state\_dump |
| PROGRAM\_DUMP | Dumps the current loaded program to 'dumped-program.obj' | 0 | program\_dump|
| MOV    |  Moves a value in to a register | 2 | mov %value, %reg | 
| LD     |  Loads the value of a register in to another | 2 | ld %reg\_from, %reg\_to |
| INC    |  Increments the value of a register by 1 | 1 | inc %reg |
| DEC    |  Decrements the value of a register by 1 | 1 | dec %reg |
| STO\_PC | Stores the imediate next OPERATION entry to a register | 1 | sto\_pc %reg |
| CMP    | Compares the values of 2 registers by subtracting reg\_b from reg\_a, then sets COMP flag accordingly | 2 | cmp %reg\_a, %reg\_b|
| JMP | Unconditional jump, sets program counter to value of register | 1 | jmp %reg|
| JE | Jump if equals. Jumps program counter to value of register if last CMP instruction yielded 0| 1 | je %reg\_jumpTo|
| JNE | Jump if not equals. Jumps program counter to value of register if last CMP instruction yielded anything other than ZERO| 1 |jne %reg\_jumpTo| 
| JGE | Jump if greater or equals. Jumps the program counter to the value of register if the last CMP instruction yielded ZERO or POSITIVE | 1 | jge %reg\_jumpTo|
| JLE | Jump if less or equals. Jumps the program counter to the value of register if the last CMP instruction yielded NEGATIVE or ZERO| 1 | jle %reg\_jumpTo |




- %reg = register index (Ex: %1, %2, %10)
- %value = any signed 32 Bit number (int32\_t)
