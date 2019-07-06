# SIC-Assembler

* Opcode.txt - contains a list of Opcodes for various **mnemonics**
* SIC.txt - contains a SIC program
* SymTab.txt - generated by program to store location of **labels**
* objectProgram.txt - the final file generated by program that contains object program of the code specified in **SIC.txt**

## How To Run:
Compile sic.c and execute the executable (SIC.txt name is hardcoded into the program for input file)

## What can it do?
It can perform some limited tasks only:
	- Location/address generation
	- OpCode Generation for each instruction
		- Pretty much all kind of instructions
	- Object Code generation

This program can not relocate code.