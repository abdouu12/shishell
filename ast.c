#include <stdio.h>
#include <stdlib.h>


struct node{

    int data;

    struct node* left;
    
    struct node* right;

};


struct node* create_node(int data, struct node* left, struct node* right){

    struct node* n = malloc(sizeof(struct node));

    n->data = data;

    n->right = right;

    n->left = left;

    return n;
}


struct node* insert_node(int insert_value, struct node* root){
    struct node* curr;
    struct node** direction  = (insert_value < root->data) ? &root->left : &root->right;

        if (* direction == NULL){

            curr = create_node(insert_value, NULL, NULL);
           *direction = curr;
            return root;
        
        }

        else{
            root = *direction;
            return insert_node(insert_value, *direction);
        
        }

}


void print_tree(struct node *root, int depth) {
    if (root == NULL)
        return;

    // Print right subtree first (so it appears at the top)
    print_tree(root->right, depth + 1);

    // Indent based on depth
    for (int i = 0; i < depth; i++)
        printf("    "); // 4 spaces per level

    // Print current node's data
    printf("%d\n", root->data);

    // Print left subtree
    print_tree(root->left, depth + 1);
}



void free_tree(){



}



int main(){

    struct node* root = create_node(10, NULL, NULL);
    insert_node(5, root);
    insert_node(15, root);
    insert_node(7, root);
    insert_node(3, root);
    insert_node(20, root);
    insert_node(25, root);

    print_tree(root, 2);
    return 0;

}







