#include "include/parser.h"
#include "include/lexer.h"
#include <string.h>

// TODO: Create data structures for each type of expression

Parser *init_parser(Lexer *lex) {

  Parser *parser = calloc(1, sizeof(Parser));
  parser->lexer = lex;
  parser->tkn_list = lex->tokens;
  parser->tkn_node = lex->tokens->head;
  parser->token = parser->tkn_node->token;

  return parser;
}

void parser_destroy(Parser *parser) {

  // Free lexer->source
  // Free lexer->token->str
  // Free lexer->tokens (individually)
  // Then free the list struct
  // Free lexer->tokenlist
  // Then finally free the parser
  free(parser);
}

// Returns the token that has been eaten and advances to next token
Token *eat(Parser *parser, TokenType type) {

  if (parser->token->type != type) {
    PRINT_ERROR("Error! Token type expected: `%s`, but received: `%s`",
                tokentype_to_string(type),
                tokentype_to_string(parser->token->type));
    exit(1);
  }

  Token *curr = parser->token;
  parser->tkn_node = parser->tkn_node->next;
  parser->token = parser->tkn_node->token;

  printf("Eaten token with type `%s`\n", tokentype_to_string(type));

  return curr;
}

AST_t *parse_statement(Parser *parser) {

  AST_t *ast = ast_create(AST_STATEMENT);
  TokenType type = parser->token->type;
  if (type == TOKEN_FOR) {
    // Handle for loops
  } else if (type == TOKEN_PRINT) {
    eat(parser, TOKEN_PRINT);
    eat(parser, TOKEN_STRING);
    // handle print statement
  }
  eat(parser, TOKEN_SEMICOLON);
  return ast;
}

AST_t *parse_identifier(Parser *parser) {

  if (parser->token->type != TOKEN_IDENTIFIER) {
    printf("No identifier found. \n");
    return NULL;
  }
  Token *token = eat(parser, TOKEN_IDENTIFIER);
  AST_t *ast = ast_create(AST_VAR);
  ast->name = token->str;
  return ast;
}

AST_t *parse_block(Parser *parser) {
  eat(parser, TOKEN_LEFT_BRACE);

  // Empty block
  if (parser->token->type == TOKEN_RIGHT_BRACE) {
    eat(parser, TOKEN_RIGHT_BRACE);
    return NULL;
  }

  AST_t *ast = ast_create(AST_COMPOUND);

  while (parser->token->type != TOKEN_RIGHT_BRACE) {
    array_push(ast->children, parse_statement(parser));
  }

  eat(parser, TOKEN_RIGHT_BRACE);

  return ast;
}

AST_t *parse_args(Parser *parser) {

  /* printf("Parsing arguments.\n"); */
  eat(parser, TOKEN_LEFTPAREN);

  if (parser->token->type == TOKEN_RIGHT_PAREN) {
    eat(parser, TOKEN_RIGHT_PAREN);
    return NULL;
  }

  AST_t *ast = ast_create(AST_COMPOUND);

  while (parser->token->type != TOKEN_RIGHT_PAREN) {
    array_push(ast->children, parse_identifier(parser));
    eat(parser, TOKEN_COMMA);
  }

  eat(parser, TOKEN_RIGHT_PAREN);

  return ast;
}

AST_t *parse_function(Parser *parser) {

  /* printf("Function being parsed.\n"); */
  AST_t *ast = ast_create(AST_COMPOUND);

  ast->name = eat(parser, TOKEN_IDENTIFIER)->str;

  // parse arguments
  AST_t *args = parse_args(parser);
  if (args) {
    array_push(ast->children, args);
  }
  AST_t *block = parse_block(parser);
  if (block) {
    array_push(ast->children, block);
  }

  return ast;
}

AST_t *parse_class(Parser *parser) {

  AST_t *ast = ast_create(AST_COMPOUND);
  /* printf("Class being parsed: `%p`\n", (void *)ast); */
  eat(parser, TOKEN_CLASS);

  if (parser->token->type == TOKEN_IDENTIFIER) {
    ast->name = parser->token->str;
  }

  eat(parser, TOKEN_IDENTIFIER);
  /* printf("Class name is: `%s`\n", ast->name); */
  eat(parser, TOKEN_LEFT_BRACE);

  while (parser->token->type != TOKEN_RIGHT_BRACE) {
    array_push(ast->children, parse_function(parser));
  }

  eat(parser, TOKEN_RIGHT_BRACE);

  return ast;
}

// NOTE: Declaration = classDeclaration | funDeclaration
//                   | varDeclaration | statement
AST_t *parse_declaration(Parser *parser) {

  /* AST_t *ast = ast_create(AST_DECLARATION); */
  AST_t *ast = ast_create(AST_COMPOUND);
  /* printf("Declaration being parsed: `%p`\n", (void *)ast); */

  // Parse class
  if (parser->token->type == TOKEN_CLASS) {
    array_push(ast->children, parse_class(parser));

  } else if (parser->token->type == TOKEN_FUNC) {
    // TODO
    eat(parser, TOKEN_FUNC);

  } else if (parser->token->type == TOKEN_VAR) {
    // TODO
    // Parse variable declaration
    eat(parser, TOKEN_VAR);

  } else {
    // TODO
    // Parse statement
    parse_statement(parser);
  }

  return ast;
}

AST_t *parse_program(Parser *parser) {

  AST_t *ast = ast_create(AST_COMPOUND);
  ast->type = AST_PROGRAM;

  PRINT_TRACE("Root: `%p`", (void *)ast);

  while (parser->token->type != TOKEN_EOF) {
    array_push(ast->children, parse_declaration(parser));
  }

  pretty_print_ast(ast, 0);

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
  printf("%s (%d)\n", node->name, node->type);

  // Recursively print the children of the node
  for (size_t i = 0; i < node->children->size; i++) {
    pretty_print_ast(node->children->items[i], depth + 1);
  }
}
