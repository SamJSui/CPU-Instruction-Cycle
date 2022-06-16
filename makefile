CC = g++
CFLAGS = -g -Wall -I include
SRC = ./src

main: assembly
	$(CC) $(CFLAGS) -o decode ./src/*.cpp

assembly: 
	nasm -f bin -o a.out ./tests/hello_world_example.asm

run: main
	./decode a.out

qemu: assembly
	qemu-system-x86_64 a.out --nographic