# Kwork project
## This page is the home for yet unfamous, but promising project **Kwork**
>So what's all about?

You may be woundering.

This project is all you need to Develop, Deploy, and Run anything and everywhere !

The project is consisted of 3 main parts:
- ### Kwork Virtual CPU
  -Capable of runing generated Kwork Assembly code
- ### Kwork programming language
  -Capable of translating high level Kwork programming language to Kwork machine code
- ### Kwork Operation System 
  -Which is completly virtualy ran by Kwork Virtual CPU
## Kwork virtual CPU
Basicly a C program which emulates work of the real CPU on low level.
Uses Kworker assembly code (.kw) and executes it continnusly. 
Has a multithreading which is built upon rapidly changing 
Kwork vitural CPU instruction pointer between all threads and preserving the state of the register.
Also supports thread locking
## Kwork Programing language
A relativly high level programing language that generates Kwork assembly code which is later assebled to Kwork CPU machine code.
Consists of Kwork compiler and Kwork assembler
## Kwork OS 
Fully emulated operation system with simple file system and with console ui.
Emulates hardware.
# Kwork project contributors section

## Kwork OS 
- [ ] Hardware emulation
  - [ ] Standart input device
  - [ ] Standart output device
  - [ ] Storage device
  - [ ] Timer device
  - [ ] signals
- [ ] Kernel
  - [ ] Memory managment
  - [ ] File system
  - [ ] Program execution
  - [ ] Signals handling
 - [ ] User space 
