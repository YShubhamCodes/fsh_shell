#ifndef UTIL_H
#define UTILS_H

#define MAX_ARGS 64
#define MAX_COMMANDS 16 // max no. of pipe allowed in one prompt

typedef struct {
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
} Command;

typedef struct {
    Command commands[MAX_COMMANDS];
    int command_count;
} Pipeline;


#endif