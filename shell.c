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