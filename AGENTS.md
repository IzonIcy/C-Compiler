# Lumi Compiler

A hand-built C compiler written in C with a real multi-stage pipeline (lexer, parser, codegen).

## Quick Start

- Build: `make`
- Run: `./build/bin/C-Compiler <source.c>`
- Clean: `make clean`
- Tests: `make test`

## Project Structure

```
├── build/              # Build output
├── include/            # Header files
├── examples/           # Example C programs
├── src/                # Compiler source
├── Makefile            # Build system
├── *.plist             # Parser/lexer/codegen metadata (plist files)
└── compile_flags.txt   # Compiler flags
```

## Pipeline

1. Lexer → Token stream
2. Parser → AST
3. Codegen → Output

## Key Notes

- Pure C, no external dependencies beyond a C compiler
- The `.plist` files contain pipeline metadata/serialized representations
- Check `Makefile` before adding new targets
