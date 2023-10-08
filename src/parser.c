#include "include/parser.h"
#include "include/lexer.h"
#include "include/token.h"
#include <stdio.h>
#include <string.h>

AST_t *parse_declaration(Parser *parser);

static size_t index = 0;

Parser *init_parser(Lexer *lex) {
  Parser *parser = calloc(1, sizeof(Parser));
  parser->lexer = lex;

  if (parser->lexer->token_list->items) {
    parser->token = (Token *)parser->lexer->token_list->items[index];
  } else {
    exit(1);
  }

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
  if (index < parser->lexer->token_list->size) {
    index++;
    Token *next = (Token *)parser->lexer->token_list->items[index];
    parser->token = next;
  } else {
    printf("Finished parsing!\n");
    exit(1);
  }

  printf("Eaten token with type `%s`\n", tokentype_to_string(type));

  return curr;
}

AST_t *parse_statement(Parser *parser) {
  AST_t *ast = ast_create(AST_STATEMENT);
  TokenType type = parser->token->type;
  if (type == TOKEN_FOR) {
    // Handle for loops
  } else if (type == TOKEN_PRINT) {

    ast->type = AST_PRINT_STMT;
    eat(parser, TOKEN_PRINT);

    /*   // TODO Finish implementing this */
    /* ast->print_stmt.print_targets = array_create(sizeof(AST_t *)); */
    /* while (parser->token->type != TOKEN_SEMICOLON) { */

    /*   array_push(ast->print_stmt.print_targets, parse_expression(parser)); */
    /* } */

    // FIXME 2023-10-08 Remove this and replace with the code above after
    // implementing expression parsing
    eat(parser, TOKEN_STRING);
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
  AST_t *ast = ast_create(AST_STRING_LIT);
  ast->str_literal.str = token;
  return ast;
}

array_T *parse_block(Parser *parser) {
  eat(parser, TOKEN_LEFT_BRACE);

  // Empty block
  if (parser->token->type == TOKEN_RIGHT_BRACE) {
    eat(parser, TOKEN_RIGHT_BRACE);
    return NULL;
  }

  array_T *blockbody = array_create(sizeof(AST_t *));
  while (parser->token->type != TOKEN_RIGHT_BRACE) {
    array_push(blockbody, parse_declaration(parser));
  }

  eat(parser, TOKEN_RIGHT_BRACE);

  return blockbody;
}

array_T *parse_args(Parser *parser) {
  /* printf("Parsing arguments.\n"); */
  eat(parser, TOKEN_LEFTPAREN);

  if (parser->token->type == TOKEN_RIGHT_PAREN) {
    eat(parser, TOKEN_RIGHT_PAREN);
    return NULL;
  }

  array_T *args = array_create(sizeof(Token *));

  while (parser->token->type != TOKEN_RIGHT_PAREN) {
    array_push(args, eat(parser, TOKEN_IDENTIFIER));
    if (parser->token->type == TOKEN_RIGHT_PAREN) {
      break;
    }
    eat(parser, TOKEN_COMMA);
  }

  eat(parser, TOKEN_RIGHT_PAREN);

  return args;
}

AST_t *parse_function(Parser *parser) {
  AST_t *ast = ast_create(AST_COMPOUND);
  ast->type = AST_FUNC_DECL;
  printf("[FUNC PTR] `%p`\n", (void *)ast);
  eat(parser, TOKEN_FUNC);

  ast->func_decl.name = eat(parser, TOKEN_IDENTIFIER);

  // parse arguments
  ast->func_decl.args = parse_args(parser);

  ast->func_decl.children = parse_block(parser);
  if (ast->func_decl.children != NULL) {
    ast->func_decl.has_body = true;
  }

  return ast;
}

AST_t *parse_class(Parser *parser) {
  AST_t *ast = ast_create(AST_COMPOUND);
  printf("[CLASS PTR] `%p`\n", (void *)ast);
  ast->type = AST_CLASS_DECL;

  /* printf("Class being parsed: `%p`\n", (void *)ast); */
  eat(parser, TOKEN_CLASS);

  Token *name = eat(parser, TOKEN_IDENTIFIER);
  ast->class_decl.name = name;

  /* printf("Class name is: `%s`\n", ast->name); */
  eat(parser, TOKEN_LEFT_BRACE);

  while (parser->token->type != TOKEN_RIGHT_BRACE) {
    array_push(ast->children, parse_function(parser));
  }

  if (ast->children != NULL) {
    ast->class_decl.has_body = 1;
  }

  eat(parser, TOKEN_RIGHT_BRACE);

  return ast;
}

// NOTE: Declaration = classDeclaration | funDeclaration
//                   | varDeclaration | statement
AST_t *parse_declaration(Parser *parser) {
  /* AST_t *ast = ast_create(AST_DECLARATION); */

  // Parse class
  switch (parser->token->type) {

  case TOKEN_CLASS: {
    return parse_class(parser);
  }
  case TOKEN_FUNC: {

    return parse_function(parser);
  }

  case TOKEN_VAR: {
    // parse var
    return NULL;
  }

  default: {

    return parse_statement(parser);
  }
  }

  return NULL;
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
