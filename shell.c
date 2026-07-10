#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv){
    //Load config files 

    //Run command loop
    lsh_loop();

    // Perform any shutdown()/cleanup()
    return EXIT_SUCCESS;
}