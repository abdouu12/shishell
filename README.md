# Shishell

Shishell is a small Unix-like shell written in C.  
It builds an abstract syntax tree (AST) for each command and executes it using system calls like `fork`, `execve`, `pipe`, and `dup2`.

## Features
- Runs commands from `$PATH`
- Supports output redirection `>` and `>>`
- Supports piping `|`
- Built-in commands: `cd`, `exit`, `history`
- Simple command history
- Proper memory cleanup

## How it works
1. The input line is split into tokens.  
2. Commands and operators are pushed to stacks.  
3. An AST is created based on operator precedence.  
4. The tree is evaluated recursively using process creation and redirection.

## Build
```bash
gcc -g -Wall -Wextra shishell.c systemcalls.c -o shishell
