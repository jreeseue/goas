#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"

unsigned int NUM_COLS;
unsigned int NUM_RECS;

/* This function initializes the memory required for a record and goes through
 each line of the file creating records and adding them to the record array.*/
void create_records(r_list *relation, char **names, char *line, FILE *fr) {
  int c_count = 0;
  char delims[2] = "\t";
  char *parsed_line = NULL;

  ssize_t bytes_read;
  size_t nbytes = 512;
  do {
    /* Initialize memory for this records. */
    record *rec = malloc(sizeof(record));
    rec->data = malloc(NUM_COLS * sizeof(char *));

    /* Make sure the allocations worked. */
    if (rec->data == NULL) {
      printf("Malloc problem in init_record().\n");
      exit(-1);
    }

    c_count = 0;
    parsed_line = strtok(line, delims);
    
    /* Add each field to the record. */
    while (parsed_line != NULL && parsed_line[0] != '\n') {
      if (parsed_line[strlen(parsed_line)-1] == '\n')
	parsed_line[strlen(parsed_line)-1] = '\0';

      rec->data[c_count] = malloc(strlen(parsed_line) * sizeof(char) + 1);
      
      if (rec->data[c_count] == NULL) {
	printf("Malloc problem in create_record().\n");
	exit(-1);
      }

      strcpy(rec->data[c_count], parsed_line);
      c_count++;

      parsed_line = strtok(NULL, delims);
    }

    /* Make sure this wasn't a blank line or some other unusual nonsense. */
    if (parsed_line == NULL) {
      add_record(relation, rec);
      free(rec);
    }

    /* Free the record if this line was junk. */
    else {
	free(rec->data);
	free(rec);
    }
  } while ((bytes_read = getline(&line, &nbytes, fr)) != -1);
  NUM_RECS = relation->rec_count;
  if (parsed_line != NULL && parsed_line[0] != '\n')
    free(parsed_line);
} /* create_record() */

/* This function initializes the memory required for a record and goes through
 each line of the file creating records and adding them to the record array.*/
/* void par_create_records(r_list *relation, char **names, char * line, FILE *fr, */
/* 			unsigned int lines) { */
/*   int c_count = 0; */
/*   int i,j; */
/*   char delims[2] = "\t"; */
/*   char *parsed_line = NULL; */
/*   int p_len; */
/*   ssize_t bytes_read; */
/*   size_t nbytes = 512; */


/*   #pragma omp parallel \ */
/*     shared(fr, names, relation)	\ */
/*     private(i, c_count, parsed_line, bytes_read, p_len, j, delims, line, nbytes) */
/*   { */
/*     #pragma omp for schedule(dynamic) */
/*     for (i=0; i<lines; i++) { */
/*       #pragma omp critical (one) */
/*       bytes_read = getline(&line, &nbytes, fr); */

/*       /\* Initialize memory for this records. *\/ */
/*       record *rec = malloc(sizeof(record)); */
/*       rec->data = malloc(NUM_COLS * sizeof(char *)); */

/*       /\* Make sure the allocations worked. *\/ */
/*       if (rec->data == NULL) { */
/*     	printf("Malloc problem in init_record().\n"); */
/*     	exit(-1); */
/*       } */
/*       c_count = 0; */

/*       parsed_line = strtok(line, delims); */

/*       /\* Add each field to the record. *\/ */
/*       while (parsed_line != NULL && parsed_line[0] != '\n') { */
/* 	p_len = strlen(parsed_line); */
/*     	if (parsed_line[p_len - 1] == '\n') */
/*     	  parsed_line[p_len - 1] = '\0'; */
/* 	rec->data[c_count] = malloc(strlen(parsed_line) * sizeof(char) + 1); */
	
/* 	/\* if (rec->data[c_count] == NULL) { *\/ */
/* 	/\*   printf("Malloc problem in create_record().\n"); *\/ */
/* 	/\*   exit(-1); *\/ */
/* 	/\* } *\/ */

/* 	/\* strcpy(rec->data[c_count], parsed_line); *\/ */
/* 	if ((rec->data[c_count] = strdup(parsed_line)) == NULL) { */
/* 	  printf("Malloc problem in create_record().\n"); */
/* 	  exit(-1); */
/* 	} */
/* 	c_count++; */
	
/*     	parsed_line = strtok(NULL, delims); */
/*       } */
      
/*       /\* Make sure this wasn't a blank line or some other unusual nonsense. *\/ */
/*       if (parsed_line == NULL) { */
/*     	#pragma omp critical (two) */
/* 	{ */
/* 	  printf("THREAD:%d rel:%p\n",omp_get_thread_num(), relation); */
/* 	  add_record(relation, rec); */
/* 	} */
/*     	free(rec); */
/*       } */
/*       /\* Free the record if this line was junk. *\/ */
/*       else { */
/*     	free(rec); */
/*     	free(rec->data); */
/*       } */
/*     } */
/*   } /\* end omp parallel *\/ */
/*   NUM_RECS = relation->rec_count; */
/*   if (parsed_line != NULL && parsed_line[0] != '\n') */
/*     free(parsed_line); */
/* } /\* create_record() *\/ */

/* Basic initialization for an r_list. Allocates memory and sets the record
 * count to 0. */
r_list *init_r_list(char **names) {
  r_list *r = malloc(sizeof(r_list));
  check_malloc(r,"init_r_list()");

  r->records = malloc(NUM_RECS * sizeof(record));
  check_malloc(r->records,"init_r_list()");

  r->c_names = malloc(NUM_COLS * sizeof(char *));
  check_malloc(r->c_names,"init_r_list()");

  unsigned int i;
  for (i=0; i<NUM_COLS; i++) {
    r->c_names[i] = malloc(strlen(names[i]) * sizeof(char));
    check_malloc(r->c_names,"init_r_list()");
    strcpy(r->c_names[i], names[i]);
  }

  r->rec_count = 0;

  return r;
} /* init_r_list() */

/* Adds record r to the list llist. */
void add_record(r_list *list, record *r) {
  int r_count = list->rec_count;

  /* Make sure there is room in the array of records. */
  if (r_count < NUM_RECS) {
    list->records[r_count] = *r;
    list->rec_count++;
  }
  /* Not enough room for this records, resize, add. */
  else {
    NUM_RECS*=2;
    list->records = realloc(list->records, sizeof(record) * NUM_RECS);
    check_malloc(list->records, "add_record()");
    add_record(list, r);
  }
} /* add_record() */

/* Prints the contents of each record in the list passed as the first
 * parameter. */
void print_r_list(r_list *list) {
  int i,j,k;
  
  /* Print column names */
  for (i=0; i<NUM_COLS; i++) {
    if (i != NUM_COLS-1)
      /* printf(" %s |",list->records[0].names[i]); */
      printf(" %s |",list->c_names[i]);
    else
      /* printf(" %s",list->records[0].names[i]); */
      printf(" %s",list->c_names[i]);
  }
  printf(" \n");
  for (i=0; i<NUM_COLS; i++) {
    for (j=0; j<strlen(list->c_names[i])+2; j++)
      printf("-",list->c_names[i]);
    if (i != NUM_COLS-1)
      printf("+");
  }
  printf("\n");
  int len;
  for (i=0; i<NUM_RECS; i++) {
    for (j=0; j<NUM_COLS; j++) {
      len = strlen(list->c_names[j])+1;
	len -= strlen(list->records[i].data[j]);
      for (k=0; k<len; k++)
	printf(" ");
      printf("%s",list->records[i].data[j]);
      if (j != NUM_COLS-1)
	printf(" |");
      free(list->records[i].data[j]);
    }
    printf("\n");
    free(list->records[i].data);
  }
  for (i=0; i<NUM_COLS; i++)
    free(list->c_names[i]);
  free(list->c_names);
  free(list->records);
  free(list);
  printf("(%d rows)\n",NUM_RECS);
  printf("\n");
} /* print_r_list() */

void check_malloc(void *array, char *errormsg) {
  if (array == NULL) {
    printf("Unable to allocate memory in %s.\n",errormsg);
    exit(-1);
  }
} /* check_malloc() */

/* This is the callback function for sorting keys. */
int int_cmp(const void *a, const void *b) {
  int *ia = (int *)a;
  int *ib = (int *)b;

  return *ia - *ib;
} /* int_cmp() */