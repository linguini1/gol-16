### Make the entire project ###

all: gasm gemu mcasm

# Assembler
gasm:
	$(MAKE) -C ./assembler assembler

# Emulator
gemu:
	$(MAKE) -C ./emulator all

# Microcode assembler
mcasm:
	$(MAKE) -C ./schematic all
