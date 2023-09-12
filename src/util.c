#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

/* Functions for file handling ***********************************************/

/* NOTE Unused struct */
typedef struct File_t {
  size_t file_size;
  char *file_contents;

} File_t;

/* Opens file and returns a pointer. */
FILE *file_open(char *filename) {

  FILE *fp = fopen(filename, "rb");

  if (fp == NULL) {
    PRINT_ERROR("FILE %s does not exist!", filename);
    print_usage();
    exit(1);
  }
  PRINT_TRACE("File '%s' opened", filename);

  return fp;
}

// Returns string of file contents
char *file_open_read(char *filename) {

  // Opening file
  FILE *filePtr = fopen(filename, "rb");

  if (filePtr == NULL) { // If error
    PRINT_ERROR("FILE %s does not exist!", filename);
    print_usage();
    exit(1);
  }

  PRINT_TRACE("File '%s' has been opened", filename);

  // Getting the filesize
  int fseekErr = fseek(filePtr, 0L, SEEK_END);

  if (fseekErr) { // If error during seeking
    PRINT_ERROR("%s", "Fseek returned err");
    fclose(filePtr);
    return NULL;
  }

  size_t filesize = ftell(filePtr);
  assert(filesize > 0 && "File is empty!");
  rewind(filePtr); // Moving cursor back to beginning of file
  PRINT_TRACE("File size is: %zu", filesize);

  // Retrieving contents of file
  char *contents =
      malloc(sizeof(char) * (filesize + 1)); // String to store the bytes read
  contents[filesize] = '\0';                 // Will terminate the char
  size_t readCount = 0; // Keeps track of the number of bytes read
  size_t loopCount = 0; // Keeps track of iterations

  // Reading file now...
  while (readCount < filesize) {

    // Exit if eof reached
    if (feof(filePtr)) {
      PRINT_TRACE("%s", "End of file reached!");
      break;
    }
    // Exit if error
    if (ferror(filePtr)) {
      PRINT_TRACE("%s", "Error has occured whilst reading the file.");
      break;
    }

    // Reading bytes
    size_t currBytesRead = fread(contents, 1, filesize - readCount, filePtr);
    readCount += currBytesRead;
    PRINT_TRACE("Loop: %zu, readCount: %zu", loopCount, readCount);
    loopCount++;
  }

  // If the number of bytes are not the same as the filesize, then output error
  if (readCount != filesize) {
    PRINT_ERROR("%s", "Not all of the file was read!");
    fclose(filePtr);
    return NULL;
  }

  fclose(filePtr); // Close file
  return contents;
}

// Returns file length
size_t file_size_name(char *filename) {

  FILE *file = file_open(filename);

  int fseekErr = fseek(file, 0L, SEEK_END);
  if (fseekErr) {
    PRINT_ERROR("%s", "fseek returned err");
    fclose(file);
    return 0;
  }
  long size = ftell(file);
  assert(size > 0 && "File is empty!");

  rewind(file);
  fclose(file);

  return size;
}

size_t file_size_ptr(FILE *fileptr) {

  int err = fseek(fileptr, 0L, SEEK_END);
  if (err) {
    PRINT_ERROR("%s", "fseek returned err");
    fclose(fileptr);
    return 0;
  }
  long size = ftell(fileptr);
  assert(size > 0 && "Fileptr is empty!");

  rewind(fileptr);
  fclose(fileptr);

  return size;
}
