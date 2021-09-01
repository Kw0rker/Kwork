all: kernel assembler compiler
A=kwork_assembler
K=kwork_kernel
C=kwork_compiler
OBJECTS=
CFLAGS = -g -Wall -O3 -Ilibs
LDLIBS = 
CC=c99

kernel:
$(K) : $(OBJECTS)	
assembler:
$(A) : $(OBJECTS)
compiler:
$(C) : $(OBJECTS)	
