#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"
#include "scan.h"

unsigned int NUM_COLS = MAX_COLS;
unsigned int NUM_RECS = MAX_RECS;

/* Scans the file specified by file_name allocates all the memory necessary
 * for a relation. When we leave this function NUM_COLS and NUM_RECS are
 * accurate measure of the actual number of columns and number of records
 * currently in the relation. */ 
r_list *scan(char *file_name) {
  /* Variables for getting the column names. */
  char **column_names;
  column_names = malloc(NUM_COLS * sizeof(char *));
  check_malloc(column_names, "scan()");
  
  char delims[2] = "\t";
  char *parsed_line = NULL;

  /* For reading from the file. */
  ssize_t bytes_read;
  size_t nbytes=80;
  char *line = NULL;

  /* FILE *fr = fopen(file_name, "r"); */
  FILE *fr = fopen(file_name, "rb");

  if (fr == NULL) {
    printf("Unable to open file [%s].\n",file_name);
    exit(-1);
  }
  struct stat sb;
  if (stat(file_name, &sb) < 0) {
    perror("fstat");
    exit(-1);
  }

  /* Take care of the first line - column names */
  bytes_read = getline(&line, &nbytes, fr);
  if (bytes_read == -1) {
    printf("File [%s] appears to be empty.\n",file_name);
    fclose(fr);
    exit(-1);
  }
  parsed_line = strtok(line, delims);
  int i = 0;
  /* strip out the column names. separated by \t */
  while (parsed_line != NULL) {
    if (i < NUM_COLS) 
      column_names[i] = malloc(strlen(parsed_line) * sizeof(char));
    else {
      NUM_COLS*=2;
      column_names = realloc(column_names, NUM_COLS * sizeof(char *));
      check_malloc(column_names, "scan()");
      column_names[i] = malloc(strlen(parsed_line) * sizeof(char));
    }
    check_malloc(column_names[i], "scan()");
    
    strcpy(column_names[i++], parsed_line);
    parsed_line = strtok(NULL, delims);
  }
  NUM_COLS = i;
  i--;

  /* Remove trailing \n */
  if (column_names[i][strlen(column_names[i])-1] == '\n')
    column_names[i][strlen(column_names[i])-1] = '\0';

  /* This will only happen if there are no records in the table. */
  bytes_read = getline(&line, &nbytes, fr);
  if (bytes_read == -1) {
    printf("This table appears to not have any entries.\n");
    fclose(fr);
    exit(-1);
  }

  /* Initialize the relation. */
  r_list *relation = init_r_list(column_names);
  /* Clean up. */
  if (parsed_line != NULL && parsed_line[0] != '\n')
    free(parsed_line);

  /* Read the lines and create a record for each entry. */
  create_records(relation, line, fr);
  
  fclose(fr);
  /* Free the memory created for the column names. */
  for (i=0; i<MAX_COLS; i++) 
    free(column_names[i]);
  free(column_names);

  return relation;
} /* scan() */