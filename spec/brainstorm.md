# Key specifications

Big-endian
16 bit data bus, 16 bit address
Load/Store architecture
Capable of driving a screen
Capable of handling interrupts

# Operations

Produce flags: Carry, Overflow, Zero, Negative

- Add
- Subtract
- Multiply
- Divide
- Logical operations

# Registers

4 GP Registers: R0-R3
Stack pointer (init to 0x0000)
Link register (init to 0x0000)
Program Counter (init to 0x0000)
Flag register (CVZN + I)

# Memory map

64KB of memory -> 0x0000 - 0xFFFF
Stack from highest memory moving down
0x0000 -> Operating system
0x0001 - 0x0003 -> Interrupt subroutines
0x0004 - top of stack -> Heap
