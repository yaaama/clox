#include "include/ast.h"
#include <stdio.h>

AST_t *ast_create(AST_Type type) {

  AST_t *ast = calloc(1, sizeof(AST_t));

  ast->type = type;

  if (type == AST_COMPOUND) {
    ast->children = array_create(sizeof(AST_t *));
  }

  return ast;
}

// Function to pretty print an AST node and its children
void pretty_print_ast(AST_t *node, int depth) {
  if (node == NULL) {
    return;
  }

  // Print indentation based on the depth of the node
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }

  // Print the node's name and type
  printf("NAME: %s , TYPE: (%d)\n", node->name, node->type);

  // Recursively print the children of the node
  for (size_t i = 0; i < node->children->size; i++) {
    pretty_print_ast(node->children->items[i], depth + 1);
  }
}
