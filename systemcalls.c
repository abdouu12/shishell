#include <asm-generic/errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <strings.h>
#include <stdlib.h>
#include "shishell.h"
#include <fcntl.h>
extern char **environ; 
#define CHILD 0
#define ERROR -1
#define max_size 100

enum precedence_table{
    pipe_precedence,
    redirection_overwrite_precedence
};


enum op_or_cmd {
    is_op,
    is_cmd
};

struct node {
    
    char op;
    char** command;
    struct node* left;
    struct node* right;

};

struct node_stack{

    struct node* arr[max_size];
    int top;
};

struct op_stack {

    char arr[max_size];
    int top;

};

struct command_stack {
    
    char* arr[max_size];
    int top;

};

struct node* create_node(char op , char** command,  struct node* left, struct node* right){

        struct node* n = malloc(sizeof(struct node));
        
        n->command = command;
        n->op = op;
        n->left = left;
        n->right = right;

        return n;



}

void initialize_command_stack(struct command_stack* s) {
    s->top = -1;
}

void initialize_op_stack(struct op_stack* op_stack){

        op_stack->top = - 1;

}
void initialize_node_stack(struct node_stack* node_stack){

        node_stack->top = -1;

}

char* pop_command(struct command_stack* s) {
    if (s->top == -1) return NULL; 
    return s->arr[s->top--];
}

struct node* pop_node(struct node_stack* s) {
    if (s->top == -1) return NULL;  
    return s->arr[s->top--];
}

char pop_op(struct op_stack* s) {
    if (s->top == -1) return '\0';  
    return s->arr[s->top--];
}


void push_command(struct command_stack* s, char* token) {
    if (s->top == max_size - 1) return; 
    s->arr[++s->top] = token;
}

void push_node(struct node_stack* s, struct node* n) {
    if (s->top == max_size - 1) return; 
    s->arr[++s->top] = n;
}

void push_op(struct op_stack* s, char op) {
    if (s->top == max_size - 1) return; 
    s->arr[++s->top] = op;
}

char* peek_command(struct command_stack* s) {
    if (s->top == -1) return NULL;
    return s->arr[s->top];
}

char peek_op(struct op_stack* s) {
    if (s->top == -1)
        return '\0';
    return s->arr[s->top];
}

struct node* peek_node(struct node_stack* s) {
    if (s->top == -1)
        return NULL;
    return s->arr[s->top];
}



void reverse_command_stack(struct command_stack* s) {
    int i = 0, j = s->top;
    while (i < j) {
        char* temp = s->arr[i];
        s->arr[i] = s->arr[j];
        s->arr[j] = temp;
        i++;
        j--;
    }
}

enum op_or_cmd op_or_cmd(char chr){
    
    if (strchr("|>", chr) != NULL){
        return is_op;
    }
    else if (strchr(">>", chr) != NULL){
        return is_op;
    }
    else{
        return is_cmd;
    }
}


enum precedence_table give_precedence_num_str(const char *op) {
    if (strcmp(op, "|") == 0)
        return pipe_precedence;
    else if (strcmp(op, ">") == 0 || strcmp(op, ">>") == 0)
        return redirection_overwrite_precedence;
    else
        return -1;
}



void command_collector(struct node_stack* node_stack, struct command_stack* command_stack) {
    if (command_stack->top == -1)
        return;

    reverse_command_stack(command_stack);

    int count = command_stack->top + 1;
    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count; i++) {
        char *token = pop_command(command_stack);
        argv[i] = strdup(token);
        if (!argv[i]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
    }
    argv[count] = NULL;

    struct node *curr = create_node(' ', argv, NULL, NULL);
    push_node(node_stack, curr);
}


struct node* make_ast(char** argv, struct op_stack* op_stack, struct node_stack* node_stack, struct command_stack* command_stack){

    int i;
    char prev_op, curr_op;
    int prev_op_precedence, curr_op_precedence; 
    struct node* curr;



    for(i = 0; argv[i] != NULL; i++){
        
        curr_op = *argv[i];
        prev_op = peek_op(op_stack);
        
        curr_op_precedence = give_precedence_num(curr_op);
        prev_op_precedence = give_precedence_num(prev_op);

        if (op_or_cmd(*argv[i]) == is_op){

            command_collector(node_stack, command_stack);
            
            while(op_stack->top != -1 && prev_op_precedence >= curr_op_precedence){
            
                curr = create_node(pop_op(op_stack),NULL ,NULL , NULL);

                curr->right = pop_node(node_stack);
                curr->left  = pop_node(node_stack);


                push_node(node_stack, curr);
                
                prev_op_precedence = give_precedence_num(peek_op(op_stack));
            }
            
            push_op(op_stack, curr_op);

        }   

        else{

            push_command(command_stack, argv[i]);
        
        }


    }

    if(command_stack->top != -1) command_collector(node_stack, command_stack);

    while(op_stack->top !=-1) {

        curr = create_node(pop_op(op_stack), NULL, NULL, NULL);
        
        curr->right = pop_node(node_stack);
        curr->left = pop_node(node_stack);

        push_node(node_stack, curr);

    }


    return pop_node(node_stack);

}

char* merge_paths(char* path,char* exe){
    size_t len = strlen(path) + 1 + strlen(exe) +1;
    char* result = malloc(len);
    strcpy(result, path);
    strcat(result, "/");
    strcat(result, exe);
    return result;
}


void execute_command(char **argv) {
    if (argv == NULL || argv[0] == NULL)
        _exit(127);

    char *path_env = getenv("PATH");
    if (!path_env) {
        fprintf(stderr, "PATH not set\n");
        _exit(127);
    }

   
    char *path_copy = strdup(path_env);
    if (!path_copy) {
        perror("strdup");
        _exit(127);
    }

    char *dir = strtok(path_copy, ":");
    while (dir) {
        char *fullpath = merge_paths(dir, argv[0]);
        execve(fullpath, argv, environ);
        free(fullpath);
        dir = strtok(NULL, ":");
    }

    fprintf(stderr, "%s: command not found\n", argv[0]);
    free(path_copy);
    _exit(127); 
}




void handle_op(struct node *parent_node) {
    struct node *left_command  = parent_node->left;
    struct node *right_command = parent_node->right;

    if (parent_node->op == '|') {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return;
        }

        pid_t left_pid = fork();
        if (left_pid == 0) {
            // Left child writes
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execute_command(left_command->command);
        } else if (left_pid < 0) {
            perror("fork");
            return;
        }

        pid_t right_pid = fork();
        if (right_pid == 0) {
            // Right child reads
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);
            execute_command(right_command->command);
        } else if (right_pid < 0) {
            perror("fork");
            return;
        }

        // Parent closes and waits
        close(fd[0]);
        close(fd[1]);
        waitpid(left_pid, NULL, 0);
        waitpid(right_pid, NULL, 0);
        return;
    }

    // Redirection (> or >>)
    const char *target = (right_command && right_command->command)
                             ? right_command->command[0]
                             : NULL;
    if (!target) {
        fprintf(stderr, "missing filename for redirection\n");
        return;
    }

    int flags = (parent_node->op == '>') ?
        (O_WRONLY | O_CREAT | O_TRUNC) :
        (O_WRONLY | O_CREAT | O_APPEND);

    int fd = open(target, flags, 0666);
    if (fd == -1) {
        perror("open");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execute_command(left_command->command);
    } else if (pid < 0) {
        perror("fork");
    } else {
        close(fd);
        waitpid(pid, NULL, 0);
    }
}




void evaluate_tree(struct node *tree_node) {
    if (!tree_node)
        return;

    
    if (tree_node->op == '|' || tree_node->op == '>' || tree_node->op == '+') {
        handle_op(tree_node);
    } else {
       
        pid_t pid = fork();
        if (pid == 0)
            execute_command(tree_node->command);
        else if (pid > 0)
            waitpid(pid, NULL, 0);
        else
            perror("fork");
    }
}



void prompt(){
    printf("(shishell)>");
}



void cd(char** argv, int argc, char* home){

        if (argc > 2){
            perror("too many args for cd command buddy");
        }

        else if (argv[1] == NULL || strcmp(argv[1], "~") == 0) {
            chdir(home);
        }
        else{
            
            chdir(argv[1]);
            }         
}








void free_node(struct node *n) {
    if (!n) return;

    free_node(n->left);
    free_node(n->right);

    if (n->command) {
        for (int i = 0; n->command[i]; i++)
            free(n->command[i]);
        free(n->command);
    }

    free(n);
}


void free_all(struct node_stack *node_stack,
              struct op_stack *op_stack,
              struct command_stack *command_stack)
{
    if (node_stack) free(node_stack);
    if (op_stack) free(op_stack);
    if (command_stack) free(command_stack);
}


void free_history(char **history_argv, int history_argc) {
    for (int i = 0; i < history_argc; i++)
        free(history_argv[i]);
    free(history_argv);
}


void get_input(char **line) {
    size_t len = 0;
    ssize_t nread = getline(line, &len, stdin);
    if (nread == -1) {
        perror("getline");
        exit(EXIT_FAILURE);
    }
    (*line)[strcspn(*line, "\n")] = '\0';
}


int main(void){ 
    welcome();
    //command line variables
    static char **argv = NULL;
    int argc = 0;

    char** history_argv = NULL; 
    int history_argc = 0;
    //get_line variables
    char* line = NULL; 
    //useful variables
    int off_set= 0;
    //enviromental variables
    char* home = getenv( "HOME" );    
    //struct declarations
    struct op_stack* op_stack = malloc(sizeof(struct op_stack));
    struct node_stack* node_stack= malloc(sizeof(struct node_stack));
    struct command_stack* command_stack = malloc(sizeof(struct command_stack));

    initialize_op_stack(op_stack);
    initialize_node_stack(node_stack);
    initialize_command_stack(command_stack);

    struct node* root;

    while(1){

    prompt();
    fflush(stdout);


    //getting input
    get_input(&line);

    //history
    history_argv = (history_argc == 0)  ? malloc(1 * (sizeof(char*))) : realloc(history_argv,20 * sizeof(char*));
    history_argv[history_argc] = malloc(strlen(line) +1);
    strcpy(history_argv[history_argc],line);
    

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
    //this is done because execve requires the last element to be NULL 
    argv = realloc(argv, (argc + 1) * sizeof(char*));
    argv[argc] = NULL;
    
    
    

    if (strcmp(argv[0], "exit") == 0) {
        free_history(history_argv, history_argc);
        free_all(node_stack, op_stack, command_stack);
        free(argv);
        free(line);
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0) {
        cd(argv,argc,home);
    }
    else if (strcmp(argv[0], "history") == 0){
        for (int element = 0; element <= history_argc; element++) {
            printf("%d: %s\n",element, history_argv[element]);
        }
    }
    else{
        node_stack->top = -1;
        command_stack->top = -1;
        op_stack->top = -1;
        root = make_ast(argv,op_stack,node_stack,command_stack);
        evaluate_tree(root);
        free_node(root);
    }

    history_argc++;
    argc = 0;
    off_set = 0;
    for (int i = 0; i < argc; i++)
            free(argv[i]);
    free(argv);
    free(line);
    line = NULL;
    }

    free_history(history_argv, history_argc);
    free_all(node_stack, op_stack, command_stack);
    return 1;
}




//implement arrow keys 
//implement tab completion
//implement cd .. and cd - 
//implement piping 







