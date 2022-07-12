
OBJECTS=
CFLAGS = -g -Wall -O0 -Ilibs -D_DEBUG
LDLIBS = 
CC=c99
all : assembler compiler kernel	

assembler : kwork_assembler.c
	$(CC) kwork_assembler.c -o build/kwork_assembler $(CFLAGS)
compiler : kwork_compiler.c
	$(CC) kwork_compiler.c -o build/kwork_compiler $(CFLAGS)
kernel : kwork_kernel.c
	$(CC) kwork_kernel.c -o build/kwork_kernel $(CFLAGS)