# SISA_assembler
C++ assembler for the SISA language

## Installation
After cloning/downloading the repo run in the same folder:

    make

## Usage
The assembler provides a handful of flags

### Options

`-p <path>` specifies the assembly file path

`-o <path>` specifies the output file (if none is provided the default is `output.bin`)

`-t <Num>`    Specifies the memory position of the first `.text` element 

`-d <Num>`    Specifies the memory position of the first `.data` element 
* if `-d` not provided, placing `.data` after `.text`
* if `-t` not provided, placing `.text` after `.data`
* if none of `-t` and `-d` provided default `-d 0x0000`

`-l <path>`    Specifies the path for the labels output file

### Flags

`-h` displays the help message

`-v`    Divides the output memory files in two (for `MEM0` and `MEM1`)

## Examples
You can find two examples in the [asm](./asm) directory.
