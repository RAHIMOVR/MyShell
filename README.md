# Simple C Shell

A basic command-line shell written in C. This project is designed to learn how operating systems handle processes, memory, and input/output.

## Features

- **Command Loop:** Continuously prompts for input until `exit` is typed or `Ctrl-D` is pressed.
- **External Commands:** Runs system commands like `ls`, `pwd`, `date`, etc.
- **Built-in Commands:** Supports `cd`, `echo`, `exit`, and `help` internally.
- **I/O Redirection:** Supports input (`<`) and output (`>`) file redirection.
- **Pipes:** Supports piping output from one command to another using `|`.
- **Logical Operators:** Supports `&&` to run a command only if the previous command succeeds and `||` to run a command only if the previous command fails.
- **Environment Variables:** Supports expanding environment variables using `$VAR`.
- **Signal Handling:** Pressing `Ctrl-C` prints a new prompt instead of closing the shell.
- **Memory Tracking:** Tracks allocated memory and displays the current allocation count when the shell exits.

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
> - Quoted arguments and arguments containing spaces
> - Multiple pipes in a single command
> - Advanced shell features such as command substitution and wildcard expansion

## License

This is a learning project. Feel free to use and modify.
