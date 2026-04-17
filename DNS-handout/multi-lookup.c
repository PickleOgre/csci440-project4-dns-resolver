/*
 * File: multi-lookup.c
 * Author: Josiah Lawrence
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2026/04/15
 * Description:
 * 	This file contains the threaded
 *      solution to this assignment.
 *
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "queue.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define NUM_THREADS 1
#define QUEUE_SIZE 10

/* Limits */
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

/*void*
request_thr(void* file)
{
}*/

typedef struct
{
  int id;
  char* hostname;
  char* firstipstr;
  FILE* outputfp;
} ResolverArgs;

void*
resolve_thr(void* arg)
{
  ResolverArgs* args = (ResolverArgs*)arg;
  /* Lookup hostname and get IP string */
  if (dnslookup(args->hostname, args->firstipstr, INET6_ADDRSTRLEN) ==
      UTIL_FAILURE) {
    fprintf(stderr, "dnslookup error: %s\n", args->hostname);
    strncpy(args->firstipstr, "", INET6_ADDRSTRLEN);
  }

  /* Write to Output File */
  fprintf(args->outputfp, "%s,%s\n", args->hostname, args->firstipstr);
  int* result = malloc(sizeof(int));
  *result = args->id * 2;
  return result;
}

int
main(int argc, char* argv[])
{
  /* Local Vars */
  FILE* inputfp = NULL;
  FILE* outputfp = NULL;
  char hostname[SBUFSIZE];
  char errorstr[SBUFSIZE];
  char firstipstr[INET6_ADDRSTRLEN];
  int i;
  pthread_t resolver_threads[NUM_THREADS];
  queue* q;

  /* Check Arguments */
  if (argc < MINARGS) {
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
    return EXIT_FAILURE;
  }

  /* Open Output File */
  outputfp = fopen(argv[(argc - 1)], "w");
  if (!outputfp) {
    perror("Error Opening Output File");
    return EXIT_FAILURE;
  }

  /* Initialize queue */
  queue_init(q, 10);

  /* Loop Through Input Files */
  for (i = 1; i < (argc - 1); i++) {

    /* Open Input File */
    inputfp = fopen(argv[i], "r");
    if (!inputfp) {
      sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
      perror(errorstr);
      break;
    }

    /* Read File and Process*/
    while (fscanf(inputfp, INPUTFS, hostname) > 0) {
      ResolverArgs args = { .id = 1,
                            .hostname = hostname,
                            .firstipstr = firstipstr,
                            .outputfp = outputfp };
      pthread_create(&resolver_threads[0], NULL, resolve_thr, &args);
      void* retval;
      pthread_join(resolver_threads[0], &retval);
      printf("Thread returned: %d\n", *(int*)retval);
      free(retval);
    }

    /* Close Input File */
    fclose(inputfp);
  }

  /* Close Output File */
  fclose(outputfp);

  /* Free queue memory */
  queue_cleanup(q);

  return EXIT_SUCCESS;
}
