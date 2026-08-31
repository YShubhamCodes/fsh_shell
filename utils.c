#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"


// List of Built-in Commands as string literals
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

// Array of matching function pointers
int (*builtin_func[]) (char **) = {
    &fsh_cd,
    &fsh_help,
    &fsh_exit
};

// Helper function to get number of built-ins
int fsh_num_builtins(void){
    return sizeof(builtin_str) / sizeof(char *);
}

// 1. Built-in 'cd' function
int fsh_cd(char **args){
    if(args[1] == NULL){
        // if user types 'cd' , default to HOME environment
        char *home = getenv("HOME");
        if(home == NULL || chdir(home) != 0){
            perror("fsh: cd expected argument");
        }
    }
    else{
        if(chdir(args[1]) != 0){
            perror("fsh");
        }
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