# A prototype assembler for the gol-16 asm language
__author__ = "Matteo Golin"

# Imports
from cmd import parser, OBJ_FILE_EXT

# Constants


# Main
def main():

    # Parse arguments
    args = vars(parser.parse_args())

    infile = args.get("i")
    outfile = args.get("o")  # Determine outfile
    if outfile is None:
        outfile = infile.with_suffix(OBJ_FILE_EXT)

    # Open the program file
    with open(infile, "r", encoding="ascii"):
        pass

    # Write to the output file
    with open(outfile, "w", encoding="ascii"):
        pass


if __name__ == '__main__':
    main()
