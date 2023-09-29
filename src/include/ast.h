#ifndef AST_H_
#define AST_H_

#include "list.h"
#include "token.h"

/* AST Types */
/* #define FOREACH_AST_TYPE(AST_TYPE) \ */
/*   AST_TYPE(AST_PROGRAM) \ */
/*   AST_TYPE(AST_NOTHING) \ */
/*   AST_TYPE(AST_COMPOUND) \ */
/*   AST_TYPE(AST_STATEMENT) \ */
/*   AST_TYPE(AST_DECLARATION) \ */
/*   AST_TYPE(AST_CLASS) \ */
/*   AST_TYPE(AST_FUNCTION) \ */
/*   AST_TYPE(AST_VAR) \ */
/*   AST_TYPE(AST_EXPR) \ */
/*   AST_TYPE(AST_ASSIGNMENT) \ */
/*   AST_TYPE(AST_BINARY) \ */
/*   AST_TYPE(AST_UNARY) \ */
/*   AST_TYPE(AST_CALL) \ */
/*   AST_TYPE(AST_PRIMARY) */

/* #define GENERATE_ENUM(ENUM) ENUM, */
/* #define GENERATE_STRING(STRING) #STRING, */

/* #define str(x) #x */
/* #define xstr(x) str(x) */

/* typedef enum { */
/*   FOREACH_AST_TYPE(GENERATE_ENUM) */

/* } AST_Type; */

typedef enum {
  AST_PROGRAM,
  AST_NOTHING,
  AST_COMPOUND,
  AST_STATEMENT,
  AST_DECLARATION,
  AST_CLASS,
  AST_FUNCTION,
  AST_VAR,
  // Expressions (produces values)
  AST_EXPR,
  AST_ASSIGNMENT,
  AST_BINARY,
  AST_UNARY,
  AST_CALL,
  AST_PRIMARY,
} AST_Type;

typedef struct AST_STRUCT {

  const char *name;
  struct AST_STRUCT *value;
  AST_Type type;
  array_T *children;

} AST_t;

// NOTE:   "Assign   : Token name, Expr value",
typedef struct {
  AST_t *expr;
  Token *name;
  AST_t *value;
} Assign;

// NOTE: "Binary   : Expr left, Token operator, Expr right",
typedef struct {
  AST_t expr;
  AST_t *left;
  Token *operator;
  AST_t *right;
} Binary;

// NOTE:  "Call     : Expr callee, Token paren, List<Expr> arguments",
typedef struct {
  AST_t expr;
  AST_t *callee;
  Token *paren; // token for the closing parenthesis; its location is used when
                // reporting a runtime error
  AST_t *arguments; // List of expressions
} Call;

// NOTE:  "Get      : Expr object, Token name",
typedef struct {
  AST_t expr;
  AST_t *object;
  Token *name;
} Get;

// NOTE:  "Grouping : Expr expression",
typedef struct {
  AST_t expr;
  AST_t *expression;
} Grouping;

// NOTE:  "Literal  : Object value",
typedef struct {
  AST_t expr;
  Token value;
} Literal;

// NOTE:  "Logical  : Expr left, Token operator, Expr right",
typedef struct {
  AST_t expr;
  AST_t *left;
  Token *operator;
  AST_t *right;
} Logical;

// NOTE: "Unary    : Token operator, AST_t right",
typedef struct {
  AST_t AST_t;
  Token *operator;
  AST_t *right;
} Unary;

// NOTE: "Variable : Token name"
typedef struct {
  AST_t expr;
  Token *name;
} Variable;

AST_t *ast_create(AST_Type type);
char *ast_type_to_str(AST_Type type); // TODO

void pretty_print_ast(AST_t *node, int depth);

AST_t *ast_assign(Token *name, AST_t *value);
AST_t *ast_binary(AST_t *left, Token *operator, AST_t * right);
AST_t *ast_call(AST_t *callee, Token *paren, AST_t *arguments);
AST_t *ast_literal(Token *value);
AST_t *ast_logical(AST_t *left, Token *operator, AST_t * right);
AST_t *ast_unary(Token *operator, AST_t * right);
AST_t *ast_variable(Token *name);
AST_t *ast_number_literal(double value);
/* AST_t *init_set(AST_t *object, Token *name, AST_t *value); */
/* AST_t *init_get(AST_t *object, Token *name); */
/* AST_t *init_grouping(AST_t *expression); */
/* AST_t *init_super(Token *keyword, Token *method); */
/* AST_t *init_this(Token *keyword); */
/* AST_t *init_bool_literal(bool value); */
/* AST_t *init_nil_literal(void); */
/* void free_expr(AST_t *AST); */

#endif // AST_H_
