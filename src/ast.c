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
void pretty_print_ast(AST_t *node, int indent) {

  if (node == NULL) {
    return;
  }

  for (int i = 0; i < indent; i++) {
    printf("  "); // Print two spaces for each level of indentation
  }

  switch (node->type) {
  case AST_PROGRAM:
    printf("[Program]\n");
    break;
  case AST_CLASS_DECL:
    printf("[Class] Name: `%s`, has_body: `%d`\n", node->class_decl.name->str,
           node->class_decl.has_body);
    break;
  case AST_FUNC_DECL: {
    printf("[Function] Name: `%s`, has_body: `%d`\n", node->func_decl.name->str,
           node->func_decl.has_body);

    if (node->func_decl.args != NULL) {
      for (size_t i = 0; i < node->func_decl.args->size; i++) {

        for (int i = 0; i < indent + 1; i++) {
          printf("  "); // Print two spaces for each level of indentation
        }
        Token *arg = (Token *)node->func_decl.args->items[i];
        printf("[Argument %zu]: %s \n", i, arg->str);
      }
    }

    if (node->func_decl.has_body) {
      for (size_t i = 0; i < node->func_decl.children->size; i++) {

        for (int i = 0; i < indent; i++) {
          printf("  "); // Print two spaces for each level of indentation
        }
        AST_t *funcBody = (AST_t *)node->func_decl.children->items[i];
        pretty_print_ast(funcBody, indent);
      }
    }

    break;
  }

  case AST_STRING_LIT:
    printf("[String literal] `%s`\n", node->str_literal.str->str);
    break;
  case AST_INT_LIT:
    printf("[Int literal] `%s`\n", node->str_literal.str->str);
    break;
  case AST_PRINT_STMT:
    printf("[Print statement] \n");

    if (node->print_stmt.print_targets != NULL) {
      for (size_t i = 0; i < node->print_stmt.print_targets->size; i++) {
        // print out the targets...
      }
    }
    break;
  default:
    printf("[OTHER] Type: `%d`\n", node->type);
    break;
  }

  /* Recursively print children */
  if (node->children != NULL) {
    for (size_t i = 0; i < node->children->size; i++) {
      pretty_print_ast(node->children->items[i],
                       indent + 1); // Increment indent for child nodes
    }
  }
}
