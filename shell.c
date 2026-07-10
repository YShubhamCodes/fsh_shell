#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


void lsh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("fsh$> ");
        line = fsh_read_line();
        args = fsh_split_line(line);
        status = fsh_execute(args);

        free(line);
        free(args);
        
    }
}


int main(int argc, char **argv){
    //Load config files 

    //Run command loop
    lsh_loop();

    // Perform any shutdown()/cleanup()
    return EXIT_SUCCESS;
}