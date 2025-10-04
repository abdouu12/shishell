#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <strings.h>
#include <stdlib.h>
#include "shishell.h"
#include <errno.h>

#define CHILD 0
#define ERROR -1

void prompt(){
    printf("(shishell)>");
}

char* merge_paths(char* path,char* exe){
    size_t len = strlen(path) + 1 + strlen(exe) +1;
    char* result = malloc(len);
    strcpy(result, path);
    strcat(result, "/");
    strcat(result, exe);
    return result;
}

void cd(char** argv, char* home){

        if (argv[1] == NULL || strcmp(argv[1], "~") == 0) {
            chdir(home);
        }
        else{
            chdir(argv[1]);
        }

}


void syscalls(char** argv) {
    char* src_$PATH = getenv( "PATH" );
    char* cpy_PATH;
    cpy_PATH = malloc(strlen(src_$PATH) +1);
    cpy_PATH = strcpy(cpy_PATH,src_$PATH);

    char * relative_path  = strtok(cpy_PATH, ":");
    char* exe = argv[0];

    int pid = fork();

    while(relative_path != NULL){

        char * merged_path = merge_paths(relative_path, exe);
     

        relative_path  = strtok(NULL, ":");
    
        if (pid == CHILD){
            execve(merged_path,argv,NULL);
            free(merged_path);            
        }
        else if (pid < 0){
            perror("fork error");   
        }
        else{
            wait(NULL);
            return;
        }
    
    }
    perror("command not found");
    free(cpy_PATH);
    

}

void get_input(char** line){
    size_t len =0;
    ssize_t nread;
    while (nread = getline(line, &len, stdin) != 1){
        if (*line)
        (*line)[strcspn(*line, "\n")] = '\0';
        return;
    }
}

int main(void){ 
    welcome();
    //command line variables
    static char **argv = NULL;
    int argc = 0;

    //get_line variables
    char* line = NULL; 
    //useful variables
    int off_set= 0;
    //enviromental variables
    char* home = getenv( "HOME" );    
    while(1){

    prompt();
    fflush(stdout);



    //getting input
    get_input(&line);


    //input tokenization
    char* t = strtok(line," ");
    argv = malloc(argc * sizeof(char*));

    while (t != NULL){
        argc++;
        argv = realloc(argv, (argc)* sizeof(char*));
        argv[off_set] = malloc(strlen(t)+1);
        strcpy(argv[off_set],t);
        off_set++;
        t = strtok(NULL," ");
    }

    argv = realloc(argv, (argc + 1) * sizeof(char*));
    argv[argc] = NULL;
    
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0) {
        cd(argv,home);
    }
    else{
    syscalls(argv);
    }
    argc = 0;
    off_set = 0;

    free(argv);
    free(line);
    }
    return 1;
}




// implemented getenv to get the PATH env dynamically
// need to fix the built in commands conlficting with syscalls (fixed)
// need to add error messages when invalid arguments are given as inputs
//







