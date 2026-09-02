# Project Name

> What do u call a fish no eyes *fsh*... bad joke, Okay just imagine I rhymed it zsh.
Fsh is a lightweight Unix shell implemented in C which supports piping, redirection and built-in commands. This was created by a student for practice...

This project explores core operating system concepts—including process forking, file descriptors, piping, and I/O redirection. Built with manual memory management and custom data structures, it features robust error handling and handles input parsing and tokenization using strtok and strtok_r.

---

## Features

- [✅] Command execution (e.g. `ls`, `pwd`, `echo`)
- [✅] Built-in commands (e.g. `cd`, `exit`, `help`)
- [✅] Piping (`|`)
- [✅] I/O redirection (`>`, `<`)
- [✅] Command history
- [⬜] Background processes (`&`)
- [⬜] Signal handling (Ctrl+C, Ctrl+Z)


I'll try to implement the ⬜ unchecked features soon. 

---

## Demo

![fsh demo](assets/demo.gif)

```sh
make clean
make
./fsh
help
cd ..
ls -lh | sort -h -r | head -n 5
curl -s wttr.in | head -n 7
echo hello > out.txt
cat < out.txt
history
```
---

## Getting Started

### Prerequisites

- GCC or any C compiler
- Make
- A Unix-like OS (Linux/macOS)

### Build

```bash
git clone https://github.com/YShubhamCodes/fsh_shell.git
cd fsh_shell
make
```

### Run

```bash
make clean
make
./fsh
```

---

## Project Structure

```
.
fsh_shell/
├── README.md
├── Makefile
├── shell.c
├── utils.c
├── utils.h
└── assets/
    └── demo.gif
```


---

## How It Works

A brief technical walkthrough of the architecture — this is what separates a "student project" from a "professional-looking project." Explain, at a high level:

- How input is read and parsed (tokenizing the command line)
##    **Line Parsing** 
- I implemented a simple parser which tokenizes using strtok_r wih standard shell delimiters. I used double tokenizers to seperate piped commands and standard commands. 
- strtok_r() is preffered over strtok() since it essentially gives up more control over pointers and thread safety
- getline() handles buffer allocation and reallocation of line. We differentiate between end-of-line exit and error exit as it returns -1 for both.

##    **Custom Structs**
- In the Command struct we have:
        * *argv array which stores pointers of the actual argument tokenized from above.
        * chat *input_file/ *ouput_file which points to file path tokenized above.
- Pipeline struct has: 
        * Command command array which stores the above custom data type command as array.
        * command_count int increamented after every command is added.

##    **Execution Logic**
- Why execution logic is needed first of all?
    If you think about it shell in itself is just an ordinary user-mode process — it has no special privilege to create or run other processes directly. To launch a program, it must ask the kernel to do that on its behalf, via system calls (fork(), execvp(), wait(), etc.). These calls trap into kernel mode, where the OS has the privilege to allocate a new process and load a program image, then return control back to user mode.

- You've probably heard that process creation in Unix needs two calls — fork() and exec(). Ever wondered why not just one?
    Most operating systems handle this with a single combined call — Windows' CreateProcess(), for example, creates a process and loads a program into it in one shot. Unix deliberately went the other way and split this into two independent system calls:

    * **`fork()`** — duplicates the calling process. The child gets a near-identical copy of the parent's entire address space (memory, open file descriptors, signal handlers, etc.). After fork(), you have two processes running the same program, differing only in the return value: 0 in the child, the child's actual PID in the parent.
    * **`exec()`** (and its variants — execvp, execve, execl, etc.) — replaces the current process's memory image with a new program, but keeps the same PID and most process metadata (open file descriptors, working directory, etc.).

    ***Why split it up at all?*** Because everything a shell needs to set up before a command runs — redirecting stdin/stdout to files or pipes, closing unused file descriptors, changing directories — happens most naturally in the gap between fork() and exec(). The child is a full, ordinary, programmable copy of the shell at that point, so fsh just rewires its own file descriptors with plain code before exec() throws that code away and drops in the real program. The new program never has to know it got redirected — it just reads fd 0 and writes fd 1 like normal.

    You can see this exact pattern in fsh_execute — every dup2() call for pipes and file redirection happens in the child, between fork() and execvp().

##    **Built-in Commands**
***Why Built-in Commands Are Separated***

Commands like `cd`, `exit`, and `history` aren't run the same way as `ls` or `grep` — they're handled directly by the shell itself, without `fork()` or `exec()` at all.

***Why?***
- **`cd`** changes the shell's own current working directory. If you ran it as a forked child process instead, only the child's directory would change — the moment that child exited, the parent shell (yours) would be right back where it started. `cd` only makes sense if it runs in the shell process, not a copy of it.
- **`exit`** has to terminate the shell itself. A forked child calling `exit()` would just kill the child — the parent shell would keep running, prompt and all.
- **`history`** reads and prints the shell's own in-memory command log — data that only exists inside the running shell process, not something a child process would have access to.

So instead of going through `fork()` + `exec()`, `fsh` checks the first word of every command against a small built-in table (`builtin_str[]` / `builtin_func[]`) before attempting to execute anything externally. If it matches, the corresponding function runs **directly in the shell process** — fast, and with the ability to actually affect the shell's own state.

```c
for(int i = 0; i < fsh_num_builtins(); i++){
    if(strcmp(pipeline->commands[0].argv[0], builtin_str[i]) == 0){
        return (*builtin_func[i])(pipeline->commands[0].argv);
    }
}
```

Everything else — programs like `ls`, `cat`, `grep` — genuinely lives on disk as separate executables, so those *do* need `fork()` + `exec()` to run.

---

## What I Learned
- Through this project, I understood a lot of OS concepts which I'll list below, If you want to fully understand this project I'll recommend going through them, I'll provide overview of some of them:
    * Process Memory Layout and Process Tables
        - Every Process believes it owns memory starting from 0x0000 and even though RAM is shared kernel translates
        Virutal address into physical address using **page tables**.

    * ElF (Executable Linkable Format)
        - It is a standard executable format on linux, It contains: 
            ~ Machine Code
            ~ Memory Layout Info
            ~ entry point
            ~ dynamic library info
        - **`execve()`** basically just replaces process image with the ELF executable for example`ls`. The old memory image disappears and new one appears.

    * Environment Variables and PATH Resolution
        - Before execution shell environment of parent is directly inherited by the child process. PATH Resolution works in order of searching as:
            ~ /usr/local/bin
            ~ /usr/bin
            ~ /bin
        - Without PATH every command would require full directory of the program ELF.

    * File Descriptors
        - It is basically a small integer that identifies an open resource owned by a process. Linux hands the application a simple integer to prevent a single file from manipulating direct memory so FDs basically acts as unified bridge between OS program and the OS's kernel.
        - It not only writes to standard text files but also to network sockets, pipes, directories and hardware devices. The default FDs are automatically populate the first three slot:
            ~ FD - 0 (Standard Input/stdin) to read data coming from keyboard.
            ~ FD - 1 (Standard Ouput/stdout) to push normal ouput text to terminal screen.
            ~ FD - 2 (Standard Error/stderr) to route error diagonistics to terminal screen.
            Every additional files,sockets or pipes opened by the program will dynamically occupy the lowest available integer starting from 3 and up.

    * Redirection and Pipes
        - They change where the standard streams (stdin, stdout, stderr) points to.
        - For example *`ls > ouput.txt`* 
            ~ stdout no longer goes to terminal
            ~ stdout goes to output.txt
        - Core idea I learned is programs shouldn't know where the ouput go, they should simply read and write from standard streams which makes the whole system reusable.
        - Pipes basically implement this `Redirection`. It is a kernel managed communication channel. It is a core inter-process communication mechanisms that channels the standard ouput(stdout) of one process directly into the standard input(stdin) of another. fsh_execute() basically handles that...

    * Signals, Process Groups, Job Control and the Terminal(TTY/PTTY).
    - **Command Lifecycle**
    ```text
    Keyboard ➔ TTY ➔ `Shell` ➔ Parse ➔ PATH Resolution ➔ Create Process ➔ Setup FD ➔ Setup Pipe ➔ Load ELF ➔ Execute ➔ Program Running
    ```

## Bugs Faced
- **Zombie Process** `wait(NULL)` wasn't looped correctly for every child in a multi-stage pipeline.
- **Pipe FD Leaks** forgetting to `close()` the unused end of a pipe in the parent of the child,
    which actually caused `cat` on the readend to hang forever waiting for EOF that never comes because a write end is still open somewhere
- **strtok_r Corruption** Earlier I had state corruption across the nested loop using the same `saveptr` across the outer pipe-splitting loop and inner space-splitting loop, which you should definately not do.
- **Segfault for execvp** non-null-terminated argv, I mentioned it in the comment under the code... Dont fall into this segfault hole like I did.
- **fsh_history loop-index bug** — used (start + 1) % HISTORY_SIZE instead of (start + i) % HISTORY_SIZE, so it kept reprinting the same slot instead of walking through the ring buffer.
- **getline/strtok_r implicit declaration under -std=c11** — compiling in strict C11 (no _POSIX_C_SOURCE defined) hid POSIX function declarations, causing GCC to assume they returned int, cascading into "makes pointer from integer without a cast" errors everywhere strtok_r's return value was used.
---

## Known Limitations / Future Improvements

- Limitations
    - **No signal handling** — Ctrl+C (SIGINT) currently kills the shell itself rather than just interrupting the foreground child, since no custom handler is installed. A real shell needs to catch SIGINT/SIGTSTP in the parent, ignore or redirect them appropriately, and let the child process receive the default behavior.
    - **No job control** — no fg, bg, jobs commands, and no SIGTSTP/SIGCONT handling to suspend and resume processes.
    - **No environment variable expansion** — $HOME, $PATH, $?, etc. aren't expanded in typed commands; only literal string arguments are passed to execvp.
    - **No globbing / wildcard expansion** — ls *.c passes the literal *.c to execvp and relies entirely on the target program to do its own glob handling (which ls doesn't do — that's normally the shell's job via glob()).
    - **No quoting support** — strtok_r splits purely on whitespace, so echo "hello world" would be parsed as two separate arguments instead of one, since there's no logic to treat quoted strings as a single token.
    - **No >> append redirection** — only > (truncate) is implemented; there's no way to append output to an existing file without overwriting it.

- Future Improvements
    - **Tab completion** : integrate readline/libedit instead of getline(), which gets you line editing, history navigation (up-arrow), and tab-completion essentially for free. Tab-Completion is single-highest leverage upgrade in this project.
    - **>> append redirection**	: Parse ">>" as a distinct token in fsh_split_line, and open with O_APPEND instead of O_TRUNC in fsh_execute. Very Easy to Implement with almost the same redirection logic.
    - **Globbing**	: Use the POSIX glob() function on each argument before building argv, so *.c expands to matching filenames like a real shell.

---

## License

This project is licensed under the [MIT License] ·

---

## Author

Shubham R. Yadav — [GitHub](https://github.com/YShubhamCodes) · 
