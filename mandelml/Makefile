
mandelml: main.cc asm.o
	g++ -Wl,-rpath,. -L. -I. -lsslib main.cc asm.o -o mandelml

asm.o: asm.asm
	nasm asm.asm -f elf64 -l asm.lst -o asm.o

clean:
	rm *.o
	rm mandelml
