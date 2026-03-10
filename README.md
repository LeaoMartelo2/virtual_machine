# virtual\_machine

Small assembly language that gets compiled to bytecode and runs on its own interpreter.

The goal of the project is to be very simple and thus be used as a learning experience.
This also means the code is not often the best, but very explicit about what it does.
I've also tried to maintain the assembly instructions with non-cryptic names, making it easier to communicate meaning.

## Compiling
The main target is `Linux x86_64` but given the very simple nature of the project, it can be compiled almost anywhere.

Requirements:
 - `gcc` (or any compiler with gnu11 support)
 - `make`

Clone the repository, and at the root of simply run:

```shell
make
```

Will result in the following bynaries:
- `vm`: the interpreter / runs the bynaries / bytecode.
- `vmasm`: the assembler / turns our assembly files in to bytecode for the vm.
- `disasm`: the disassembler / allows you to peek inside the `.bin` files, and see a approximation of their pre-assembled code

## Examples:

Please refer to `example_programs/` directory for more in depth info

### Example program:

Minimal program that makes a loop by incrementing a value and comparing it until its bigger than the compared value

Starting at the root of the project;

```asm 
# you can leave comments with a hash 
toggle_verbose 1              # enable verbose output on the machine 
mov 10, $10                   # stores '10' in the register 10
loop_start:                   # define a Label where the loop starts
inc $1                        # increments the value at register 1 by 1 ($1++)
cmp $1, $10                   # compares register 1 with register 10
jle .loop_start               # goes to the start of the loop of the comparasion was less or equal
state_dump                    # displays some information about the machine 
halt                          # finishes machine execution and exits
```

save this to `my_program.asm`

Assemble it with:
```shell 
./vmasm my_program.asm -o my_bytecode.bin
```

Then run the interpreter:
```shell
./vm my_bytecode.bin
```

Alternatively use the `-run` flag on the assembler
```shell 
./vmasm my_program.asm -run
```

Some info about the `-run` flag

- It expects the interpreter binary (`vm`) to be in the same directory, so when installing system-wide make sure they are togather.
- This will leave the default `out.bin` file, unless specified by `-o`
- This also allows you to have a `shebang` at the start of your `.asm` files, by simply adding `#!./vmasm -run` (assuming at the project root

The second to last instruction (`state_dump`) prints some info about the registers to the screen, it should look something like this:
(Click to expand)
<details closed>
<summary>state\_dump output</summary>


```raw
STATE_DUMP:
##############################
REGISTERS:
        register[ 1] = 11
[...]
        register[10] = 10
[...]
```


</details>

Now take a look at your compiled binary code by running:
```shell
./disasm my_bytecode.bin
```

Optionally add the `-s` flag to output the disassembled code to `disassembled.asm`


## Documentation

### Opcodes implemented

| Opcode |  INFO | Argument count | Example |
| -------| ------|----------------|---------|
| NO\_OP |  No operation. | 0 | no\_op|
| HALT   |  Halts the machine. | 0 | halt |
| STATE\_DUMP | Prints the value of the registers, program size and program counter | 0 | state\_dump |
| REGISTER\_DUMP | Prints the value of the registers arg\_ until arg\_b, inclusive| 2 | register\_dump $arg\_a, $arg\_b |
| PROGRAM\_DUMP | Dumps the current loaded program to 'dumped-program.bin' (useful when debugging a self altering program) | 0 | program\_dump|
| STACK\_DUMP| Prints the value at current stack header, and 3 surrounding values | 0 | stack\_dump|
| TOGGLE\_VERBOSE | Toggles verbose output of the machine on or off if `value > 0`, starts off | 1 | toggle\_verbose %value|
| MOV    |  Moves a value in to a register | 2 | mov %value, $reg | 
| LD     |  Loads the value of a register in to another | 2 | ld $reg\_from, $reg\_to |
| INC    |  Increments the value of a register by 1 | 1 | inc $reg |
| DEC    |  Decrements the value of a register by 1 | 1 | dec $reg |
| STO\_PC | Stores the imediate next OPERATION entry to a register, mostly deprecated in favor of labels | 1 | sto\_pc $reg |
| CMP    | Compares the values of 2 registers by subtracting reg\_b from reg\_a, then sets COMP flag accordingly | 2 | cmp $reg\_a, $reg\_b|
| JMP | Unconditional jump, sets program counter to value | 1 | jmp %value/label|
| JE | Jump if equals. Jumps program counter to value if last CMP instruction yielded 0| 1 | je %value/label|
| JNE | Jump if not equals. Jumps program counter to value if last CMP instruction yielded anything other than ZERO| 1 |jne %value/label| 
| JGE | Jump if greater or equals. Jumps the program counter to value if the last CMP instruction yielded ZERO or POSITIVE | 1 | jge %value/label|
| JLE | Jump if less or equals. Jumps the program counter to value if the last CMP instruction yielded NEGATIVE or ZERO| 1 | jle %value/label|
| ADD | Adds reg\_a and reg\_b, stores result to reg\_a | 2 | add $reg\_a $reg\_b |
| SUB | Subtract reg\_a and reg\_b, stores result to reg\_a | 2 | sub $reg\_a $reg\_b |
| MUL | Multiply reg\_a and reg\_b, stores result to reg\_a | 2 | mul $reg\_a $reg\_b |
| DIV | Divide reg\_a and reg\_b, stores result to reg\_a | 2 | div $reg\_a $reg\_b |
| MOD | Gives division remainder (modulo) of reg\_a and reg\_b, stores result to reg\_a | 2 | mod $reg\_a $reg\_b |
| PUSH | Pushes the value of a register in to the stack | 1 | push $reg |
| I\_PUSH | Pushes an imediate value to the stack | 1 | i\_push %value |
| POP | Pops the last item in the stack, saves it to a reg\_a | 1 | pop $reg\_a |
| VOID\_POP | Pops the last item in the stack, discarding its value| 0 | void\_pop |
| CALL | Jumps to a label and sets the return address stack | 1 | call %value/label |
| RET | Returns by jumping to the last return address stack, pops its value | 0 | ret |



- $reg = register index (Ex: $1, $2, $10)
- %value = any signed 32 Bit number (int32\_t), for certain instructions, could be replaced by a `Label`
- There are a few named registers, these being `$arg_a` .. `$arg_d`, and `$ret`, conventionally used to store arguments and return values for `call`



