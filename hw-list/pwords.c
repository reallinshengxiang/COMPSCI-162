/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

pthread_mutex_t lock;
typedef struct {
  word_count_list_t *word_counts;
  FILE *file;
} thread_args_t;

/* Function executed by each thread to count words in a file */
void* thread_count_words(void* arg) {
  thread_args_t *args = (thread_args_t *)arg;
  pthread_mutex_lock(&lock);
  count_words(args->word_counts, args->file); 
  pthread_mutex_unlock(&lock);
  fclose(args->file); 
  return NULL;
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  /* Create the empty data structure for word counts */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread if no file is provided */
    count_words(&word_counts, stdin);
  } else {
    /* TODO: Create a thread for each input file */
    pthread_t threads[argc-1]; 
    thread_args_t thread_args[argc-1];
    for (int i=1;i<argc;i++) {
      FILE *file=fopen(argv[i], "r");
      if (file==NULL) {
        perror("fopen"); 
        exit(EXIT_FAILURE);
      }
      thread_args[i-1].word_counts=&word_counts;
      thread_args[i-1].file=file;
      if (pthread_create(&threads[i-1],NULL,thread_count_words,&thread_args[i-1])!=0){
        perror("pthread_create");
        exit(EXIT_FAILURE);
      }
    }
    for (int i=1;i<argc;i++) {
      if (pthread_join(threads[i-1],NULL)!=0) {
        perror("pthread_join");
        exit(EXIT_FAILURE);
      }
    }
  }

  /* Output the final result of all threads' work */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  
  return 0;
}