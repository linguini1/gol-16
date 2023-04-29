# Contains preprocessing for the assembler
__author__ = "Matteo Golin"

# Imports
import re

# Constants
COMMENT: str = "//"


# Functions
def remove_comments(content: str) -> str:
    """Removes all comments from the assembly code."""
