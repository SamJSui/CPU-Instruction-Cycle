# **CPU Instruction Cycle**

## **Description**

A C++ program written to simulate a CPU and its instruction cycle:
- Fetch
- Decode
- Execute
- Memory
- Writeback

### Built With
- C++
- Assembly

---

## **Getting Started**

### Prerequisites
- g++ 
    - `sudo apt install g++`
- make
    - `sudo apt install make`
- nasm
    - `sudo apt install nasm`
- qemu (optional)
    - `sudo apt install aqemu`

### Install

1. Clone the repo
    - `git clone https://github.com/SamJSui/CPU-Instruction-Cycle`
2. cd into the directory 
    - `cd CPU-Instruction-Cycle`

## **Usage**

- `make` 
    - Compiles the `.cpp` files in `/src/`
- `make run`
    - Compiles the `.cpp` files in `/src/`, assembles the `.asm` file(s) in `/tests/`, and runs the program
        - By default, the makefile is set to assemble `/tests/hello_world_example.asm` into `a.out`
- `make qemu`
    - Assembles the `.asm` file(s) in `/tests/` and runs the program through qemu, a machine's processor emulator
        - With the case of the `hello_world_example`, `a.out` becomes a boot sector that prints: "Hello, World!"

### Supported Instructions

- inc
- mov
- cmp
- jmp
- int
- je

### Notes

- **x86 Assembly**
    - Moving an immediate into a 16-Bit Register will be treated like an address