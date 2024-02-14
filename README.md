# A Virtual Machine 🧮
An efficient interpreter for a simple stack-based VM that could serve as the compilation target and runtime of a programming language. Implemented in the GNU C dialect.

## Features
- Dynamic Code Reloading 👀
- Copying Grabage Collector 🥵
- Directly Threaded 💀
- Combined Instructions
- Performance Oriented Implementation
- Prefetching
- Works with Unicode

## Other Repositories Implementing the same VM 😡
- https://github.com/steve-anunknown/just-a-vm (without dynamic reloading)

## More resources 📚
- It was inspired by [this spec](https://courses.softlab.ntua.gr/pl2/2020b/exercises/vm-gc.pdf) but expanded upon
- A disassembler exists in the previously mentioned [repository](https://github.com/steve-anunknown/just-a-vm/blob/main/turn_to_assembly.c) but it won't fully work for the byte-code here as I've made some additions to the instruction set.
- An assembler generator which can be used to create an assembler for the byte-code (and was partially created for this purpose) can be found [here](https://github.com/steve-anunknown/bytecode-assembler-generator)

## Installation  ⚗
```shell
make -j2
```
produces the vm executable and
```shell
make clean
```
removes the object files

*(`make dev -j2` is intented for testing and development and the executable produced is slower 🧪)*

### Uninstall 🥺
```shell 
make distclean
```

## Examples 🔢
> [!TIP]
> - see [vm.h](vm.h) for byte-code mnemonics, like `0x04` = `SWAP` : *if the following byte's value is i, swaps the top of the stack with the element which is i bytes below it*
> - The byte-code is little-endian for constants larger than one byte
```shell
$ echo -n '\x08\x0a\x08\x82\x08\x98\x08\x9F\x08\xF0\x18\x18\x18\x18\x18' > unicode.b
$ ./vm unicode.b
😂
```

```shell
$ echo -n '\x08\x0a\x08r\x08e\x08l\x08z\x08z\x08i\x08r\x08 \x08e\x08h\x08t\x08 \x08r\x08o\x08f\x08 \x08t\x08a\x08y\x08g\x08 \x08r\x08u\x08o\x08y\x08 \x08t\x08u\x08o\x08 \x08g\x08n\x08i\x08k\x08c\x08i\x08t\x08S\x08\x27\x04\x01\x18\x08\x01\x0a\x03\x00\x02\x50\x00\x00' > hello.b
$ ./vm hello.b
Sticking out your gyat for the rizzler
```

The next example is a program that asks the user for one character of input. If that character is `r` it dynamically reloads itself (and starts execution from the begining). If it's any other character it halts.
```shell
$ echo -n '\x08r\x17\x0e\x02\x08\x00\x00\x17\x40' > reload.b
$ ./vm reload.b
r
r
e
```
