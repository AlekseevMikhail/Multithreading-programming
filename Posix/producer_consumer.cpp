#include "producer_consumer.h"
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

using namespace std;
int get_tid() {
  atomic_int tid_mem = 0;
  thread_local unique_ptr<int> tid = make_unique<int>(-1);
  if (*tid == -1) {
    tid = make_unique<int>(tid_mem.fetch_add(1));
  }
  return *tid;
}

void* producer_routine(void* arg) {
  producer_args* args = (producer_args*)arg;

  std::string line;
  std::getline(std::cin, line);
  std::istringstream in(line, std::istringstream::in);
  int n;
  while (in >> n) {
    pthread_mutex_lock(args->number_mutex);
    *args->number_ptr = n;
    *args->new_number = true;
    pthread_cond_signal(args->producer_cond);
    while (*args->new_number) {
      pthread_cond_wait(args->consumer_cond, args->number_mutex);
    }
    pthread_mutex_unlock(args->number_mutex);
  }

  pthread_mutex_lock(args->number_mutex);
  *args->completed = true;
  pthread_cond_broadcast(args->producer_cond);
  pthread_mutex_unlock(args->number_mutex);

  return nullptr;
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  consumer_args* args = (consumer_args*)arg;

  // for every update issued by producer, read the value and add to sum
  while (!*args->completed) {
    pthread_mutex_lock(args->number_mutex);
    while (!*args->new_number and !*args->completed) {
      pthread_cond_wait(args->producer_cond, args->number_mutex);
    }
    if (*args->completed) {
      pthread_mutex_unlock(args->number_mutex);
      break;
    }
    *args->new_number = false;
    args->psum += *args->number_ptr;
    pthread_cond_signal(args->consumer_cond);
    pthread_mutex_unlock(args->number_mutex);
    if (args->debug) printf("(%d, %d)\n", get_tid(), args->psum);
    if (args->ms > 0) {
      int ms = rand() % args->ms;
      if (ms > 0) {
        usleep(ms * 1000);
      }
    }
  }

  // return pointer to result (for particular consumer)
  return (void*)args;
}

void* consumer_interrupter_routine(void* arg) {
  interrupter_args* args = (interrupter_args*)arg;

  // interrupt random consumer while producer is running
  while (!*args->completed) {
    int i = 0;
    if (args->n > 1) {
      i = rand() % (args->n - 1);
    }
    pthread_cancel(args->consumers[i]);
  }
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads(unsigned short int N, unsigned short int ms, bool debug) {
  // start N threads and wait until they're done
  int number;
  bool completed = false;
  bool consumer_started = false;
  bool new_number = false;
  int amount = 0;
  int status;
  pthread_mutex_t number_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
  pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

  pthread_t producer;
  producer_args p_args;
  p_args.number_ptr = &number;
  p_args.n = N;
  p_args.completed = &completed;
  p_args.consumer_started = &consumer_started;
  p_args.new_number = &new_number;
  p_args.number_mutex = &number_mutex;
  p_args.producer_cond = &producer_cond;
  p_args.consumer_cond = &consumer_cond;
  status = pthread_create(&producer, NULL, producer_routine, (void*)&p_args);
  if (status != 0) {
    printf("error: can't create producer thread, status = %d\n", status);
    exit(ERROR_CREATE_THREAD);
  }

  pthread_t* consumers = new pthread_t[N];
  consumer_args* c_args = new consumer_args[N];
  consumer_args consumers_result;
  for (int i = 0; i < N; i++) {
    c_args[i].debug = debug;
    c_args[i].completed = &completed;
    c_args[i].consumer_started = &consumer_started;
    c_args[i].new_number = &new_number;
    c_args[i].ms = ms;
    c_args[i].psum = 0;
    c_args[i].tid = 4 + i;
    c_args[i].number_ptr = &number;
    c_args[i].number_mutex = &number_mutex;
    c_args[i].producer_cond = &producer_cond;
    c_args[i].consumer_cond = &consumer_cond;

    status = pthread_create(&consumers[i], NULL, consumer_routine,
                            (void*)&c_args[i]);
    if (status != 0) {
      printf("error: can't create consumers[%d] thread, status = %d\n", i,
             status);
      exit(ERROR_CREATE_THREAD);
    }
  }

  pthread_t interrupter;
  interrupter_args_tag i_args;
  i_args.consumers = consumers;
  i_args.n = N;
  i_args.completed = &completed;
  i_args.consumer_started = &consumer_started;
  i_args.number_mutex = &number_mutex;
  i_args.consumer_cond = &consumer_cond;
  status = pthread_create(&interrupter, NULL, consumer_interrupter_routine,
                          (void*)&i_args);
  if (status != 0) {
    printf("error: can't create interrupter thread, status = %d\n", status);
    exit(ERROR_CREATE_THREAD);
  }

  status = pthread_join(interrupter, NULL);
  if (status != 0) {
    printf("error: can't join interrupter thread, status = %d\n", status);
    exit(ERROR_JOIN_THREAD);
  }

  // return aggregated sum of values
  for (int i = 0; i < N; i++) {
    status = pthread_join(consumers[i], (void**)&consumers_result);
    if (status != 0) {
      printf("error: can't join consumers[%d] thread, status = %d\n", i,
             status);
      exit(ERROR_JOIN_THREAD);
    }
    amount += c_args[i].psum;
  }

  delete[] consumers;
  delete[] c_args;
  pthread_mutex_destroy(&number_mutex);
  pthread_cond_destroy(&producer_cond);
  pthread_cond_destroy(&consumer_cond);

  return amount;
}
