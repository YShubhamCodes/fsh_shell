#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "utils.h"

char *history[HISTORY_SIZE];
int history_index = 0;
int history_count = 0;


// List of Built-in Commands as string literals
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "history"
};

// Array of matching function pointers
int (*builtin_func[]) (char **) = {
    &fsh_cd,
    &fsh_help,
    &fsh_exit,
    &fsh_history
};

// Helper function to get number of built-ins
int fsh_num_builtins(void){
    return sizeof(builtin_str) / sizeof(char *);
}

// 1. Built-in 'cd' function
int fsh_cd(char **args){
    char *target_dir = args[1];
    if(target_dir == NULL){
        // if user types 'cd' , default to HOME environment
        target_dir = getenv("HOME");
        if(target_dir == NULL){
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    if(chdir(target_dir) != 0){
        perror("cd");
        return 1;
    }

    char *cwd = getcwd(NULL, 0);
    if(cwd != NULL){
        printf("%s\n", cwd);
        free(cwd);
    }
    else{
        perror("cd: getcwd error");
    }
    return 1;
}

// 2. Built-in 'help' (Print instructions)
int fsh_help(char **args){
    printf("--- Friendly Shell (fsh) Help ---\n");
    printf("Type program names and arguments, then hit enter. \n");
    printf("The following commands are built-in:\n");

    for(int i = 0; i < fsh_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the '|' operator to pipe commands, and '<' or '>' for redirection.\n");
    return 1;
}

int fsh_exit(char **args){
    return 0;
}

void add_to_history(char *line){
    if(line == NULL || line[0] == '\0') return;

    if(history[history_index] != NULL){
        free(history[history_index]);
    }

    history[history_index] = strdup(line);
    history_index = (history_index + 1) % HISTORY_SIZE;
    history_count++;
}

int fsh_history(char **args){
    int start = (history_count < HISTORY_SIZE) ? 0 : history_index;
    int n = (history_count < HISTORY_SIZE) ? history_count : HISTORY_SIZE;
    
    for(int i = 0; i < n; i++){
        int idx = (start + i) % HISTORY_SIZE;
        printf("%4d  %s", i + 1, history[idx]);
    }
    return 1;
}

void free_history(){
    for (int i = 0; i < HISTORY_SIZE; i++){
        if(history[i] != NULL) free(history[i]);
    }
}