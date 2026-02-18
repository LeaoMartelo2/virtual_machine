# virtual\_machine

Simple attempt at implementing a virtual machine and a bytecode language that runs on it.

In the future i will be implementing a way to assemble human readable code in to the machine's bytecode.


### Opcodes implemented

| Opcode |  INFO | Argument count |
| -------| ------|----------------|
| NO\_OP |  No operation. | 0 |
| HALT   |  Halts the machine. | 0 |
| STATE\_DUMP | Prints the value of the registers, program size and program counter | 0 |
| MOV    |  Moves a value in to a register | 2 |
| LD     |  Loads the value of a register in to another | 2 |
| INC    |  Increments the value of a register by 1 | 1 |
| STO\_PC | Stores program counter to a register | 1 |
| JMP | Unconditional jump, sets program counter to value of register | 1 |

