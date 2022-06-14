#ifndef CPU_H
#define CPU_H

#include "machine.h"

// As copied from Wikipedia

// 8 General Purpose Registers:
// Accumulator register (AX). Used in arithmetic operations
// Counter register (CX). Used in shift/rotate instructions and loops.
// Data register (DX). Used in arithmetic operations and I/O operations.
// Base register (BX). Used as a pointer to data (located in segment register DS, when in segmented mode).
// Stack Pointer register (SP). Pointer to the top of the stack.
// Stack Base Pointer register (BP). Used to point to the base of the stack.
// Source Index register (SI). Used as a pointer to a source in stream operations.
// Destination Index register (DI). Used as a pointer to a destination in stream operations.

// 6 Segment Register:
// Stack Segment (SS). Pointer to the stack ('S' stands for 'Stack').
// Code Segment (CS). Pointer to the code ('C' stands for 'Code').
// Data Segment (DS). Pointer to the data ('D' comes after 'C').
// Extra Segment (ES). Pointer to extra data ('E' stands for 'Extra').
// F Segment (FS). Pointer to more extra data ('F' comes after 'E').
// G Segment (GS). Pointer to still more extra data ('G' comes after 'F').

// EFlags Register. Collection of bits representing Boolean values to store the results of operations.
// Instruction Pointer (IP). Contains address of the next instruction.


#endif