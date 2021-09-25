
A=kwork_assembler
K=kwork_kernel
C=kwork_compiler
OBJECTS=
CFLAGS = -g -Wall -O0 -Ilibs
LDLIBS = 
CC=c99
$(K) : $(OBJECTS)

