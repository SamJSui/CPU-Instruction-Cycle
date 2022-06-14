CC = g++
CFLAGS = -g -Wall -I include
SRC = ./src

main: assembly
	$(CC) $(CFLAGS) -o decode ./src/*.cpp

assembly: 
	nasm -f elf64 -o binary ./tests/helloworld.asm

run: main
	./decode binary

debug: assembly
	objdump -d binary