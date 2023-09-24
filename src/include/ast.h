#ifndef AST_H_
#define AST_H_

#include "list.h"

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

AST_t *ast_create(AST_Type type);
char *ast_type_to_str(AST_Type type);

#endif // AST_H_
