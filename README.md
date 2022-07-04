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
## Kwork Lang
- [ ] Compiler 
  - [x] basic expressions support
  - [ ] floating numbers math and support
  - [x] string literals support
  - [ ] static type checks
  - [ ] compile time checks
  - [ ] compile time execution
  - [ ] preprocessor
  - [ ] optimization
  - [x] functions
  - [x] loops
  - [x] conditional statements
  - [x] basic input and output
  - [x] arrays
  - [x] pointers
  - [x] bitwise operations
  - [ ] structs
  - [x] static memory
  - [ ] dynamic memory
- [x] Assembler
  - [x] All operations  supported
## Virtual cpu
 - [x] threading and locks
 - [ ] floating point numbers support
 - [x] basic math and jump instuctions
 - [x] bitwise instructions
 - [x] pointers
 - [x] basic system calls
 - [x] stack   
  
## Kwork Lang Documentation
Kwork lang is multiplatform static JET compiled programming language with dynamic data types and every variable is static and global in the specific frame. There is NO semicolons so only one command can be in the line (does not include nested calls ex function calls params).
Every program must start with main function declaration following by end keyword at the end of the frame
Every variable name is one char length! 
function main

… your code 

end
every key word is caps sensitive 

Existing features:
•	Variable declaration
let lvalue = rvalue 
rValue can be const/var/array/immutable string/expression
1.	let a = “Hello world\n"
2.	let a = b
3.	let a = [20][20]
4.	let D = b*b-(4*a*c)
lValue can be array subscription/memory dereferencing/one char letter
1.	let a[2] = 1337
2.	let a[B]….[Bn] = 69
3.	let @(lvalue) = 101 
4.	let D = “This language is trash\a\n”




•	loops
loops can be for loop/
1.	for lvalue=rvalue;rvalue;rvalue
{
…. Your code
}
First rvalue is starting value
Second rvalue is expression which is true or false 1/0
Third rvalue is increment
for a=0;a<10;a++
{
….. do smt
}
•	putc 
puts char from var to stdout
(supports only var as input rn)
•	put 
puts number from var to stdout
(supports only var as input rn)

•	branches
(supports only vars as input rn)

1.	if statement
compares 2 variables
if a<b
{
…your code
}
2.	else if statement
compares 2 variable only if previous if or else if frame failed
else if a>=b
{
…your code
}
3.	else statement
executed only if all others if or else if frames failed
else
{
….your code here
}
•	input 
reads from stdin to variable
•	functions declaration
function “Name_of_you function” (arg1,arg2,…,argN)
…your code
return rvalue

•	rvalue expression
CALL “your function name” {rvalue1,rvalue2,…,rvalueN}
Will call a function with parameters
o	binary operands
1.	supports all basic math operations + * - / %
	
let a = (b%10)*(10/3+(5-7))
	
2.	supports all basic bitwise operations  && || ^ >> <<
	
&& - >binary and
	
|| -> binary or
	
^ ->binary XOR
	
>> -> bit shift right
	
<< -> bit shift left
	
3.	logical operations < > <= >=
	
o	unary operations
4.	++ -- @ # !
	
! -> bit inverse
	
++ -> increment value by one
			
-- -> decrement value by one
	
@ -> dereference the pointer
	
# -> get a pointer
	
•	Comments 
	
// your comment
Pointer can be any value even the const
let @(10) = 3010 
this line would change the 10th assembly instruction to the instruction of summing acc with the assembly instruction. 
Lets make self-modifying code great again.
In this language as you can see basically no checks on what you do with memory so quite easy to shoot yourself in the foot 😊
Best of luck !!




