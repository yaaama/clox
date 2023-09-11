#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <time.h>

/*****************************************************************************/
/*                                   Macros                                  */
/*****************************************************************************/
#define DEBUG 1
#define TRACING 1

// Macro to print an error to stderr
#define PRINT_ERROR(format, ...)                                               \
  do {                                                                         \
    if (DEBUG)                                                                 \
      fprintf(stderr, "ERROR->%s:%d: %s(): " format "\n", __FILE__, __LINE__,  \
              __func__, __VA_ARGS__);                                          \
  } while (0)

/* #define PRINT_ERR(...) \ */
/*   do { \ */
/*     ISSUE_ERROR("%s", __VA_ARGS__); \ */
/*   } while (0) */

// For tracing
#define PRINT_TRACE(format, ...)                                               \
  do {                                                                         \
    if (TRACING)                                                               \
      fprintf(stderr, "TRACE-> %s:%d: %s(): " format "\n", __FILE__, __LINE__, \
              __func__, __VA_ARGS__);                                          \
  } while (0)

/* #define PRINT_TRACE(...) \ */
/*   do { \ */
/*     ISSUE_TRACE("%s", __VA_ARGS__); \ */
/*   } while (0) */

// Debugging message one liner
#define PUT_TRACE(msg)                                                         \
  do {                                                                         \
    if (TRACING)                                                               \
      fprintf(stderr, "TRACE->%s:%d: %s(): " msg "\n", __FILE__, __LINE__,     \
              __func__);                                                       \
  } while (0)

// Macro to quickly create an error
#define ERR_CREATE(name, type, msg) Error(name) = {(type), (msg)}

/*****************************************************************************/
/*                              Type definitions                             */
/*****************************************************************************/

typedef struct Error {
  enum error_type {
    NONE,     // No error
    SYNTAX,   // There is something wrong with the syntax
    TYPE_ERR, // There is incorrect usage of types
    MISC,     // Unspecified error
  } error_type;
  char *msg; // Error msg (NULL if no error)
} Error;

static Error err_ok = {.error_type = NONE, .msg = NULL};

/*****************************************************************************/
/*                             Exposed functions                             */
/*****************************************************************************/

char *file_open_read(char *filename);
void print_usage(void);

#endif // UTIL_H_
