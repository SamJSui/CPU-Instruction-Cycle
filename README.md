<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

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
- nasm
    - `sudo apt install nasm`

### Install

1. Clone the repo
    - `git clone https://github.com/SamJSui/CPU-Instruction-Cycle`
2. cd into the directory 
    - `cd CPU-Instruction-Cycle`

### Usage

- `make` 
    - Compiles the `.cpp` files in `/src/`
- `make run`
    - Compiles the `.cpp` files in `/src/`, assembles the `.asm` file(s) in `/tests/`, and runs the program
- `make debug`
    - Assembles the `.asm` file(s) in `/tests/` and provides the instructions from the output file in hex, printing it to the terminal