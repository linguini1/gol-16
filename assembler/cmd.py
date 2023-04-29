# The commandline argument parser for the prototype assembler
__author__ = "Matteo Golin"

# Imports
import argparse
from pathlib import Path

# Constants
ASM_FILE_EXT: str = ".orgasm"
OBJ_FILE_EXT: str = ".obj"
DESCRIPTION: str = "A commandline tool for assembling gol-16 assembly language into binary."


# Custom types
def ORGASM(_parser: argparse.ArgumentParser, filepath: str) -> Path:
    """Returns the filepath if it is a valid 'original gol-16 asm' file that exists."""

    path = Path(filepath)

    if not (path.exists() and path.is_file()):
        _parser.error(f"{filepath} not found.")

    if not path.name.endswith(ASM_FILE_EXT):
        _parser.error(f"{filepath} not {ASM_FILE_EXT} format.")

    return path


def OBJFile(_parser: argparse.ArgumentParser, filepath: str) -> Path:
    """Ensures that the provided filepath is a valid .obj filepath."""

    path = Path(filepath)

    if not path.name.endswith(OBJ_FILE_EXT):
        parser.error(f"{filepath} is not a {OBJ_FILE_EXT} file.")

    if not path.parent.exists():
        parser.error(f"{filepath} parent directory does not exist.")

    return path


# Parser
parser = argparse.ArgumentParser(
    description=DESCRIPTION
)

# Commands
parser.add_argument(
    "i",
    help="The input gol-16 asm file.",
    nargs=1,
    type=lambda x: ORGASM(parser, x),
)

parser.add_argument(
    "-o",
    help="The output file for gol-16 asm binary.",
    nargs=1,
    type=lambda x: OBJFile(parser, x),
    default=None  # Default to same name and location as input file .obj
)

parser.add_argument(
    "-p",
    help="Prints the created binary to the console when created.",
    action="store_true"
)
