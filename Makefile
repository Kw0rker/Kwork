
OBJECTS=
CFLAGS = -g -Wall -O3 -Ilibs
LDLIBS = 
CC=c99
all : build_folder assembler compiler kernel	

assembler : kwork_assembler.c
	$(CC) kwork_assembler.c -o build/kwork_assembler $(CFLAGS)
compiler : kwork_compiler.c
	$(CC) kwork_compiler.c -o build/kwork_compiler $(CFLAGS)
kernel : kwork_kernel.c
	$(CC) kwork_kernel.c -o build/kwork_kernel $(CFLAGS)
build_folder :
	mkdir -p build	