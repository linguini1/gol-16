# Assembly Instructions

OpCode length: 5b
Register length: 2b
Total different instructions: 32
Total possible instructions: 32

## Arithmetic

All use unsigned imm

- `ADD rd, r, r/imm7`
- `SUB rd, r, r/imm7`
- `MUL rd, r, r/imm7`
- `DIV rd, r, r/imm7`
- `MOD rd, r, r/imm7`

## Logical

All use unsigned imm

- `AND rd, r, r/imm7`
- `OR rd, r, r/imm7`
- `NOT rd, r/imm9`

## Memory

- `MOV rd, r/imm9` (unsigned imm)
- `MVN rd, imm9` (signed imm)

All use signed imm:

- `LDR rd, [imm9]` (PC-Relative)
- `LDR rd, [r, r/imm7]` (offset register/immediate)
- `STR rs, [imm9]` (PC-Relative)
- `STR rs, [r, r/imm7]`
- `LEA rs, imm9` (PC-Relative)

LDR and STR in the form: `LDR rd, [r, #0]` will be handled internally by the processor. It will detect an immediate with a value of 0 and instead of performing the add operation it will just load directly from the address.

## Control Flow

- `CMP r, r/imm9` (unsigned imm)
- `CMN r, r/imm9` (unsigned imm representing a negative)
  - `CMN r, #3` = compare [r] with -3
- `CALLcc imm7` (signed imm)

Can take R0-R3 as arguments + PC, LR, FR:

- `PUSH {r, ...}`
- `POP {r, ...}`

Arguments are converted into a bitmask where the LSB is FR:
PUSH {R0, R3} -> uext(1001000)

PUSH: low to high registers, PC, LR, FR
POP: FR, LR, PC and high to low registers (reverse order)
Programmer does not need to change order for POP, it will be handled by the processor

### Condition Codes

- eq: Equal
- ne: Not equal
- hs: Higher or same (unsigned)
- hi: Higher (unsigned)
- lo: Lower than (unsigned)
- ls: Lower or same (unsigned)
- mi: Negative
- pl: Positive or 0
- vs: Overflow
- vc: No overflow
- ge: Greater than or equal to (signed)
- lt: Lower than (signed)
- gt: Greater than (signed)
- le: Less than or equal to (signed)
- al: Always

# No-op Instructions

`label DCD imm16`
`label EQU imm16`

# Other Syntax

Binary: `0b010...`
Hex: `0x0123...`
Character: `'A'` -> ascii encoding
String: `"Hello world"` -> sequence of DCDs
Strings will be compact (1 word will contain 2 characters)
