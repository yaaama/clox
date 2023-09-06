#include "util.h"

/* Returns local time. Useful for runtime debugging. */
const char *get_local_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

/* NOTE Revisit this function when the name of the project is decided */
void print_usage(void) { printf("Usage: %s  <file-to-compile>\n", "<nicer> "); }
