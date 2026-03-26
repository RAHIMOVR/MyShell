# Simple C Shell

A basic command-line shell written in C. This project is designed to learn how operating systems handle processes, memory, and input/output.

## Features

- **Command Loop:** Continuously prompts for input until `exit` is typed.
- **External Commands:** Runs system commands like `ls`, `pwd`, `date`, etc.
- **Built-in Commands:** Supports `cd`, `echo`, and `exit` internally.
- **I/O Redirection:** Supports input (`<`) and output (`>`) file redirection.
- **Signal Handling:** Pressing `Ctrl-C` prints a new prompt instead of closing the shell.
- **Memory Safety:** All memory is properly freed on exit. Verified with Valgrind.

## Requirements

- GCC Compiler
- Make
- Valgrind *(optional, for memory testing)*

## How to Build
```bash
make
```

## How to Run
```bash
./myshell
```

## Testing for Memory Leaks

To verify there are no memory leaks, run:
```bash
make valgrind
```

## Current Limitations

> ⚠️ The following features are not yet supported:
> - Pipes (`|`)
> - Environment variable expansion (`$VAR`)
> - Logical operators (`&&`, `||`)

## License

This is a learning project. Feel free to use and modify.
