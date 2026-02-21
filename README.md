# virtual\_machine

Simple attempt at implementing a virtual machine and a bytecode language that runs on it.

## Compiling
The main target is `Linux x86_64` but given the very simple nature of the project, it can be compiled almost anywhere.

Clone the repository, and at the root of it run:

```shell
make
```
Yes, this is literally the only thing you need to do to build the project


Will result in the following bynaries being generated:
- `vm`: the interpreter / runs the bynaries / bytecode.
- `vmasm`: the assembler / turns our assembly files in to bytecode for the vm.
- `disasm`: the disassembler / allows you to read `.obj` bytecode files without running them, and turn them back in to their `.asm` form.

## Examples:

Please refer to `example_programs/` directory for more info for now.

### Example program:

Minimal program that makes a loop by incrementing a value and comparing it until its bigger than the compared value

```asm
mov 10, %10
sto_pc %0
inc %1
cmp %1, %10
jle %0
state_dump
halt
```
save this to `my_program.asm`

Assemble it with:
```shell 
./vmasm my_program.asm -o my_bytecode.obj
```

Then run the interpreter:
```shell
./vm my_bytecode.obj
```

The second to last instruction (`state_dump`) prints some info about the registers to the screen, it should look something like this:
<details closed>
<summary>`state_dump output`</summary>



```raw
STATE_DUMP:
##############################
REGISTERS:
        register[ 0] =  5
        register[ 1] = 11
        register[ 2] =  0
        register[ 3] =  0
        register[ 4] =  0
        register[ 5] =  0
        register[ 6] =  0
        register[ 7] =  0
        register[ 8] =  0
        register[ 9] =  0
        register[10] = 10
##############################
MACHINE INFO:
        program_couter  = 12
        program_size    = 14
        condition_flag  = 1
##############################
```


</details>

Now take a look at your compiled object code by running:
```shell
./disasm my_bytecode.obj
```

## Documentation

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
