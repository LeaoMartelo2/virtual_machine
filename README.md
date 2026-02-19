# virtual\_machine

Simple attempt at implementing a virtual machine and a bytecode language that runs on it.

In the future i will be implementing a way to assemble human readable code in to the machine's bytecode.


### Opcodes implemented

| Opcode |  INFO | Argument count | Example |
| -------| ------|----------------|---------|
| NO\_OP |  No operation. | 0 | |
| HALT   |  Halts the machine. | 0 | |
| STATE\_DUMP | Prints the value of the registers, program size and program counter | 0 | |
| MOV    |  Moves a value in to a register | 2 | mov %value reg% | 
| LD     |  Loads the value of a register in to another | 2 | ld reg\_from% reg\_to% |
| INC    |  Increments the value of a register by 1 | 1 | inc reg% |
| STO\_PC | Stores program counter to a register | 1 | sto\_pc reg% |
| JMP | Unconditional jump, sets program counter to value of register | 1 | jmp reg%|
| JE | Jump if equals, sets program counter to value of register if value = 0 | 2 | je reg\_value% reg\_jumpTo% |
| JNE | Jump if not equals, sets program counter to value of register if value != 0 | 2 |jne reg\_value% reg\_jumpTo% | 




- % = number, usually indicating register index
- %value = any signed 32 Bit number (uint32\_t)
