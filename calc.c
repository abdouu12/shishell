#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define max_size 100



char *add_spaces(char *p) {
    size_t n = strlen(p);
    if (n == 0) return strdup("");

    // each char except last gets a space, +1 for '\0'
    char *out = malloc(n * 2);  
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        out[j++] = p[i];
        if (i < n - 1)
            out[j++] = ' ';
    }
    out[j] = '\0';
    return out;
}

double compute(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0; // avoid division by zero
        default:
            fprintf(stderr, "Error: Unknown operator '%c'\n", op);
            return 0.0;
    }
}
enum opstate {
    IS_OP,
    ISNT_OP
};

enum precedence_table {
    ADD_PRECEDENCE,
    SUB_PRECEDENCE,
    MUL_PRECEDENCE,
    DIV_PRECEDENCE
};
struct node_stack {
    struct node* arr[max_size];
    int top;
};

struct op_stack {
    char arr[max_size];
    int top;
};

struct node_stack* initialize_node_stack(void) {
    struct node_stack* s = malloc(sizeof(struct node_stack));
    if (!s) exit(EXIT_FAILURE);
    s->top = -1;
    return s;
}

struct op_stack* initialize_op_stack(void) {
    struct op_stack* s = malloc(sizeof(struct op_stack));
    if (!s) exit(EXIT_FAILURE);
    s->top = -1;
    return s;
}

void push_op(struct op_stack* s, char op) {
    s->arr[++s->top] = op;
}

char pop_op(struct op_stack* s) {
    return s->arr[s->top--];
}

void push_node(struct node_stack* s, struct node* value) {
    s->arr[++s->top] = value;
}

struct node* pop_node(struct node_stack *s) {
    return s->arr[s->top--];
}

struct node* peek_node(const struct node_stack *s) {
    return s->arr[s->top];
}

char peek_op(const struct op_stack *s) {
    return s->arr[s->top];
}
struct node {

    double (*func)(double, double);

    char op;
    int num;
    struct node* left;
    struct node* right;

};




enum opstate is_op(char* ch) {

    if (strchr("+-*/", *ch) != NULL) {return IS_OP;}

    return ISNT_OP;

}

enum precedence_table storing_precedence(char* chr){

    if      (strchr("+", *chr) != NULL) {return ADD_PRECEDENCE;}
    else if (strchr("-", *chr) != NULL) {return SUB_PRECEDENCE;}
    else if (strchr("*", *chr) != NULL) {return MUL_PRECEDENCE;}
    else if (strchr("/", *chr) != NULL) {return DIV_PRECEDENCE;}
    

    return -1;

}

struct node* create_node(char op, int num, struct node* left, struct node* right){
    
    struct node* n = malloc(sizeof(struct node));

    n->op = op;
    n->num = num;
    n->left = left;
    n->right = right;

    return n;

}


struct node* make_tree(char** argv, struct op_stack* op_stack, struct node_stack* node_stack){
        
    int i = 0;
    char curr_op, prev_op;
    int tmp, curr_op_precedence, prev_op_precedence;
    struct node* curr;
    
    curr = create_node(' ', atoi(argv[i]), NULL,NULL);
    push_node(node_stack,curr);
    i+=1;
    curr_op = *argv[i];
    push_op(op_stack, curr_op);
    
    i = 2;
    while (argv[i] != NULL){
        
        if (is_op(argv[i]) == IS_OP){
                curr_op = *argv[i]; //the current operator
                tmp = peek_op(op_stack);
                prev_op = (char)tmp; //the next operator
                curr_op_precedence = storing_precedence(argv[i]);
                prev_op_precedence= storing_precedence(&prev_op);
                if(curr_op_precedence >= prev_op_precedence){
                        push_op(op_stack,curr_op);
                        prev_op = curr_op; 
                        i++;
                }
                else if (curr_op_precedence < prev_op_precedence){
                    curr = create_node(pop_op(op_stack), 0, NULL,NULL);

                    curr->right = pop_node(node_stack); //note you'll ahve to change this for dividion
                    curr->left = pop_node(node_stack);
                    push_node(node_stack, curr);
                    push_op(op_stack, curr_op);
                    prev_op = curr_op;
                    i++;
                }

  
        }
        else{
                curr = create_node(' ', atoi(argv[i]),NULL, NULL);
                push_node(node_stack,curr);
                i++;
        }

      
    }
    while (op_stack->top != -1) {
        if (node_stack->top < 1) break;
    curr = create_node(pop_op(op_stack), 0, NULL,NULL);
    curr->right = pop_node(node_stack);
    curr->left = pop_node(node_stack);
    push_node(node_stack, curr);
    }
    return pop_node(node_stack);

}



char *get_input(void) {

    char *buffer = NULL;
    size_t size = 0;

    ssize_t characters = getline(&buffer, &size, stdin);

    if (characters == -1) {
        perror("getline failed");
        free(buffer);
        return NULL;
    }

    return buffer;  

}
char** parse_input(int* argc, char** argv,char* line){

    int offset = 0;
    char* t = strtok(line, " ");

    while (t != NULL){
        (*argc)++;
        argv = realloc(argv,(*argc + 1) *sizeof(char*));
        argv[offset] = malloc(strlen(t) +1);
        strcpy(argv[offset], t);

        offset++;
        t = strtok(NULL, " ");
    }
    argv[*argc] = NULL;
    return argv;

}




void print_ast(struct node* root, int depth) {
    if (root == NULL)
        return;

    // Print right subtree first (so it shows on top visually)
    print_ast(root->right, depth + 1);

    // Indentation for current level
    for (int i = 0; i < depth; i++)
        printf("    ");

    // Print current node: operator or number
    if (root->op != ' ')
        printf("%c\n", root->op);
    else
        printf("%d\n", root->num);

    // Print left subtree
    print_ast(root->left, depth + 1);
}

void free_all(){

}

int main(){
    char* line = NULL;
    char** argv = NULL;
    int argc = 0;
    struct op_stack* op_stack = initialize_op_stack();
    struct node_stack* node_stack = initialize_node_stack();
    line = get_input();
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n'){

    line[len - 1] = '\0'; 
    }

   // line = add_spaces(line);
    argv = parse_input(&argc, argv, line);

    struct node* root = make_tree(argv, op_stack, node_stack);
    print_ast(root, 2);
    


    //reiitalizing argc to 0
    argc = 0;

    return 0;
}
