
mandelml: main.cc asm.o
	g++ main.cc asm.o -o mandelmlf -lX11 -lpthread -Wl,-rpath=/usr/local/lib64

asm.o: asm.asm
	nasm asm.asm -f elf64 -l asm.lst -o asm.o

clean:
	rm *.o
	rm mandelmlf
