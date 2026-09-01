#ifndef UTILS_H
#define UTILS_H

#define MAX_ARGS 64
#define MAX_COMMANDS 16 // max no. of pipe allowed in one prompt
#define HISTORY_SIZE 50

typedef struct {
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
} Command;

typedef struct {
    Command commands[MAX_COMMANDS];
    int command_count;
} Pipeline;

// Function Declarations for shell.c
char* fsh_read_line(void);
void fsh_split_line(char* line, Pipeline *pipeline);
int fsh_execute(Pipeline *pipeline);

// Declare external variables if shell.c needs to read them
extern char *builtin_str[];
extern int (*builtin_func[]) (char **);
extern char *history[HISTORY_SIZE];
extern int history_count;
extern int history_index; // rounded buffer type


// Function Declarations for Built-in Shell Commands
int fsh_cd(char **args);
int fsh_help(char **args);
int fsh_exit(char **args);
int fsh_num_builtins(void);
void add_to_history(char *line);
int fsh_history(char **args);
void free_history(void);



#endif