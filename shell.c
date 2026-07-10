#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"


void lsh_loop(void){
    char *line;
    Pipeline pipe_line;
    int status;

    do{
        printf("fsh$> ");
        line = fsh_read_line();

        //parse the line into the Pipeline structure
        fsh_split_line(line, &pipe_line);

        status = fsh_execute(&pipe_line);

        // Freeing after execution of prompt
        free(line);

    } while(status);
}



int main(int argc, char **argv){
    //Load config files 

    //Run command loop
    lsh_loop();

    // Perform any shutdown()/cleanup()
    return EXIT_SUCCESS;
}

char* fsh_read_line(void){
    char *line;
    ssize_t bufsize = 0; // getline() will allocate a buffer

    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)) {
            exit(EXIT_FAILURE);
        }
        else{
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

void fsh_split_line(char* line, Pipeline *pipeline){
    pipeline->command_count = 0;

    char *pipe_saveptr;
    // Split the entire line by the pipe character '|'
    char *cmd_segment = strtok_r(line, "|", &pipe_saveptr);

    while(cmd_segment != NULL && pipeline->command_count < MAX_COMMANDS) {
        // Get a pointer to the current struct block we are building
        Command *current_cmd = &pipeline->commands[pipeline->command_count];

        // Initialize struct fields
        current_cmd->input_file = NULL;
        current_cmd->output_file = NULL;
        int arg_idx = 0;

        char *arg_saveptr;
        // Split the current segment by spaces to isolate words
        char *token = strtok_r(cmd_segment, " \t\r\n\a", &arg_saveptr);

        while(token != NULL){
            if(strcmp(token, "<") == 0) {
                // Next token is the input file
                token = strtok_r(NULL, " \t\r\n\a", &arg_saveptr);
                if(token != NULL){
                    current_cmd->input_file = token;
                }
            }
            else if(strcmp(token, ">") == 0){
                // Next token is the output file
                token = strtok_r(NULL, " \t\r\n\a", &arg_saveptr);
                if(token != NULL){
                    current_cmd->output_file = token;
                }
            }
            else{
                // standard command argument
                if(arg_idx < MAX_ARGS - 1){
                    current_cmd->argv[arg_idx++] = token;
                }
            }
            // Move to the next token in this segment
            token = strtok_r(NULL, " \t\r\n\a", &arg_saveptr);
        }
        // Explicitly NULL terminate the argv array after the prompt/segment
        current_cmd->argv[arg_idx] = NULL;

        // increament command count
        if(arg_idx > 0){
            pipeline->command_count++;
        }

        // Move to the next pipe segment
        cmd_segment = strtok_r(NULL, "|", &pipe_saveptr);
    }
}