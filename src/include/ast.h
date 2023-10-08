#ifndef AST_H_
#define AST_H_

#include "list.h"
#include "token.h"
#include <stdbool.h>

typedef enum {
  AST_PROGRAM = 0,
  AST_COMPOUND = 1,
  AST_CLASS_DECL = 2,
  AST_FUNC_DECL = 3,
  AST_PRINT_STMT = 4,
  AST_STRING_LIT = 5,
  AST_INT_LIT = 6,
  AST_STATEMENT,
  AST_DECLARATION,
  AST_BLOCK,
  AST_VAR,
  // Expressions (produces values)
  AST_EXPR,
  AST_ASSIGNMENT,
  AST_BINARY,
  AST_UNARY,
  AST_CALL,
  AST_PRIMARY,
  AST_NOTHING,
} AST_Type;

typedef struct AST_STRUCT {
  AST_Type type;
  array_T *children;

  union {

    struct {
      Token *name;
      bool has_body;
      array_T *children;

    } class_decl;

    struct {
      Token *name;
      bool has_body;
      array_T *args;
      array_T *children;

    } func_decl;

    struct {
      Token *name;
      bool has_val;
      struct AST_STRUCT *value;
    } var_decl;

    struct {
      array_T *print_targets; // A list of AST structs
    } print_stmt;

    struct {
      Token *str;
    } str_literal;

    struct {
      Token *num;
    } int_literal;
  };

} AST_t;

/* // NOTE:   "Assign   : Token name, Expr value", */
/* typedef struct { */
/*   Token *name; */
/*   union { */
/*     char *val; */
/*     double num_val; */
/*   }; */
/* } AST_Assign; */

/* // NOTE: "Binary   : Expr left, Token operator, Expr right", */
/* typedef struct { */
/*   AST_t *left; */
/*   Token *operator; */
/*   AST_t *right; */
/* } Binary; */

/* // NOTE:  "Call     : Expr callee, Token paren, List<Expr> arguments", */
/* typedef struct { */
/*   AST_t expr; */
/*   AST_t *callee; */
/*   Token *paren; // token for the closing parenthesis; its location is used
 * when */
/*                 // reporting a runtime error */
/*   AST_t *arguments; // List of expressions */
/* } Call; */

/* // NOTE:  "Get      : Expr object, Token name", */
/* typedef struct { */
/*   AST_t expr; */
/*   AST_t *object; */
/*   Token *name; */
/* } Get; */

/* // NOTE:  "Grouping : Expr expression", */
/* typedef struct { */
/*   AST_t expr; */
/*   AST_t *expression; */
/* } Grouping; */

/* // NOTE:  "Literal  : Object value", */
/* typedef struct { */
/*   AST_t expr; */
/*   Token value; */
/* } Literal; */

/* // NOTE:  "Logical  : Expr left, Token operator, Expr right", */
/* typedef struct { */
/*   AST_t expr; */
/*   AST_t *left; */
/*   Token *operator; */
/*   AST_t *right; */
/* } Logical; */

/* // NOTE: "Unary    : Token operator, AST_t right", */
/* typedef struct { */
/*   AST_t AST_t; */
/*   Token *operator; */
/*   AST_t *right; */
/* } Unary; */

/* // NOTE: "Variable : Token name" */
/* typedef struct { */
/*   AST_t expr; */
/*   Token *name; */
/* } Variable; */

/* typedef struct AST_t { */

/*   const char *name; */
/*   AST_t *value; */
/*   AST_Type type; */
/*   array_T *children; */

/* } AST_t; */

AST_t *ast_create(AST_Type type);
char *ast_type_to_str(AST_Type type); // TODO

void pretty_print_ast(AST_t *node, int depth);

/* AST_t *ast_assign(Token *name, AST_t *value); */
/* AST_t *ast_binary(AST_t *left, Token *operator, AST_t * right); */
/* AST_t *ast_call(AST_t *callee, Token *paren, AST_t *arguments); */
/* AST_t *ast_literal(Token *value); */
/* AST_t *ast_logical(AST_t *left, Token *operator, AST_t * right); */
/* AST_t *ast_unary(Token *operator, AST_t * right); */
/* AST_t *ast_variable(Token *name); */
/* AST_t *ast_number_literal(double value); */
/* AST_t *init_set(AST_t *object, Token *name, AST_t *value); */
/* AST_t *init_get(AST_t *object, Token *name); */
/* AST_t *init_grouping(AST_t *expression); */
/* AST_t *init_super(Token *keyword, Token *method); */
/* AST_t *init_this(Token *keyword); */
/* AST_t *init_bool_literal(bool value); */
/* AST_t *init_nil_literal(void); */
/* void free_expr(AST_t *AST); */

#endif // AST_H_
