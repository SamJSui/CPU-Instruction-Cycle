CC = g++
CFLAGS = -g -Wall -I include
SRC = ./src

main: assembly
	$(CC) $(CFLAGS) -o decode ./src/*.cpp

assembly: 
	nasm -f bin -o a.out ./tests/helloworld.asm

run: main
	./decode a.out

debug: assembly
	objdump -d a.out

qemu: assembly
	qemu-system-x86_64 a.out --nographic