#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

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
    (void)argc;
    (void)argv;
    //Load config files 

    //Run command loop
    lsh_loop();

    // Perform any shutdown()/cleanup()
    return EXIT_SUCCESS;
}

char* fsh_read_line(void){
    char *line;
    size_t bufsize = 0; // getline() will allocate a buffer

    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)) {
            exit(EXIT_FAILURE);
        }
        else{
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    add_to_history(line);
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
        // if not then the kernel would try to read beyond your argv array which will lead to segmentation fault and hell lotta crashes. This tells the kernel to stop.
        current_cmd->argv[arg_idx] = NULL;

        // increament command count
        if(arg_idx > 0){
            pipeline->command_count++;
        }

        // Move to the next pipe segment
        cmd_segment = strtok_r(NULL, "|", &pipe_saveptr);
    }
}

int fsh_execute(Pipeline *pipeline){
    if(pipeline->command_count == 0){
        return 1; // No command entered
    }

    //Built-in command check; if first command is exit, then close
    if(pipeline->commands[0].argv[0] != NULL){
        for(int i = 0; i < fsh_num_builtins(); i++){
            if(strcmp(pipeline->commands[0].argv[0], builtin_str[i]) == 0){
                // Execute the built-in function directly in parent process
                return(*builtin_func[i])(pipeline->commands[0].argv);
            }
        }
    }

    // We keep track of the read end of the previous pipe stage
    // For the very first command, there is no previous pipe
    int prev_pipe_read_fd = STDIN_FILENO; // It stores the readend value from the previous pipe iteration

    // Loop through every command stage in our pipeline array
    for(int i = 0; i < pipeline->command_count; i++){
        Command *cmd = &pipeline->commands[i]; // shortcut

        // pipefds[0] is the read end, after pipe() sys call
        // pipefds[1] is the write end, after pipe() sys call
        int pipefds[2];

        int has_next_pipe = (i < pipeline->command_count -1); // Stores a bool value, can be of boolean data style

        // if there is next command stage, create a new UNIX pipe
        if(has_next_pipe){
            if(pipe(pipefds) < 0){
                perror("fsh: pipe creation failed");
                return 1;
            }
        }
        pid_t pid = fork();
        if(pid == 0){
            // CHILD PROCESS
            // ===================== LOT OF WORK TO DO ======================
            // 1. Link pipeline Input: if this isn't the first command, grab input from the last pipe
            if(i > 0){
                dup2(prev_pipe_read_fd, STDIN_FILENO);
                close(prev_pipe_read_fd);
            }

            // 2. Link Pipeline ouput: if there is a next command, pipe stdout into it
            if(has_next_pipe){
                close(pipefds[0]); // child process created pipe. only writing to next pipe; readend not needed
                dup2(pipefds[1], STDOUT_FILENO);
                close(pipefds[1]);
            }

            // 3. Apply file redirections: Overwrite pipe mapping its explicit redirection 
            if(cmd->input_file != NULL){
                int fd_in = open(cmd->input_file, O_RDONLY);
                if(fd_in < 0){
                    perror("fsh: input file error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            if(cmd->output_file != NULL){
                int fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(fd_out < 0){
                    perror("fsh: output file error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            // 4. Fire Process Execution
            if(execvp(cmd->argv[0], cmd->argv) == -1){
                perror("fsh: execution error");
            }
            exit(EXIT_FAILURE);
        }
        else if(pid < 0){
            perror("fsh: fork failed");
            return 1;
        }
        else{
            // PARENT PROCESS (SHELL MANAGER)
            // Clean up tracking file descriptors in the parent shell loop
            if(i > 0){
                close(prev_pipe_read_fd);
            }
            if(has_next_pipe){
                close(pipefds[1]);
                prev_pipe_read_fd = pipefds[0]; // important linking line; it links the currents pipe read end and stores the data in the prev_pipe_read_fd for next iteration
            }
        }
    }

    // After starting all the forks in the pipeline, shell parent waits for the child process
    while(wait(NULL) > 0);

    return 1; // Keep shell loop alive
}

