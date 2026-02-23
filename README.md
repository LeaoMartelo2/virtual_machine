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
# you can leave comments with a hash 
mov 10, $10
loop_start:
# you can adress registers with both '%' and '$'
inc $1
cmp $1, $10
jle .loop_start
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
[...]
        register[10] = 10
[...]
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
| MOV    |  Moves a value in to a register | 2 | mov %value, $reg | 
| LD     |  Loads the value of a register in to another | 2 | ld $reg\_from, $reg\_to |
| INC    |  Increments the value of a register by 1 | 1 | inc $reg |
| DEC    |  Decrements the value of a register by 1 | 1 | dec $reg |
| STO\_PC | Stores the imediate next OPERATION entry to a register | 1 | sto\_pc $reg |
| CMP    | Compares the values of 2 registers by subtracting reg\_b from reg\_a, then sets COMP flag accordingly | 2 | cmp $reg\_a, $reg\_b|
| JMP | Unconditional jump, sets program counter to value | 1 | jmp $reg|
| JE | Jump if equals. Jumps program counter to value if last CMP instruction yielded 0| 1 | je %value|
| JNE | Jump if not equals. Jumps program counter to value if last CMP instruction yielded anything other than ZERO| 1 |jne %value| 
| JGE | Jump if greater or equals. Jumps the program counter to value if the last CMP instruction yielded ZERO or POSITIVE | 1 | jge %value|
| JLE | Jump if less or equals. Jumps the program counter to value if the last CMP instruction yielded NEGATIVE or ZERO| 1 | jle %value|
| ADD | Adds reg\_a and reg\_b, stores result to reg\_a | 2 | add $reg\_a $reg\_b |
| SUB | Subtract reg\_a and reg\_b, stores result to reg\_a | 2 | sub $reg\_a $reg\_b |
| MUL | Multiply reg\_a and reg\_b, stores result to reg\_a | 2 | mul $reg\_a $reg\_b |
| DIV | Divide reg\_a and reg\_b, stores result to reg\_a | 2 | div $reg\_a $reg\_b |
| MOD | Gives division remainder (modulo) of reg\_a and reg\_b, stores result to reg\_a | 2 | mod $reg\_a $reg\_b |

- $reg = register index (Ex: $1, $2, $10)
- %value = any signed 32 Bit number (int32\_t), for certain instructions, could be replaced by a `Label`
