#include "mutexes.h"
#include <pthread.h>
#include <signal.h>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>

#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)

pthread_mutex_t mutex;
/*
Task 1. Implement an increment function that accepts a number and increments it
thread safely

Useful links:
static variables: https://www.geeksforgeeks.org/static-variables-in-c/
pthread mutex: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-110.html
*/
void thread_safe_increment(int& value) {
  pthread_mutex_lock(&mutex);
  value++;
  pthread_mutex_unlock(&mutex);
}

/*
Task 2. Implement the transfer of the value from the provide function to the
consume function using conditional variables for notification. The context_t
implementation is in mutexes.h.

Useful links:
pthread mutex: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-110.html
pthread condition variables:
https://docs.oracle.com/cd/E19455-01/806-5257/6je9h032r/index.html
*/

/*
Steps (provide):
1. Lock the mutex from ctx.mtx
2. Set the value to ctx.shared_value
3. Put the ctx.is_provided readiness flag of the set value
4. Send a notification of readiness via pthread_cond_signal
5. Unlock the mutex from ctx.mtx
*/
void provide(context_t& ctx, int value) {
  pthread_mutex_lock(&ctx.mtx);
  ctx.shared_value = value;
  ctx.is_provided = true;
  pthread_cond_signal(&ctx.cond);
  pthread_mutex_unlock(&ctx.mtx);
}

/*
Steps (consume):
1. Lock the mutex from ctx.mtx
2. In the loop, wait when the ctx.is_provided variable is not true and call
pthread_cond_wait to wait
3. Unlock the mutex from ctx.mtx
4. Return the ctx.shared_value value from the function
*/
int consume(context_t& ctx) {
  pthread_mutex_lock(&ctx.mtx);
  while (ctx.is_provided == false) {
    pthread_cond_wait(&ctx.cond, &ctx.mtx);
  }
  pthread_cond_signal(&ctx.cond);
  pthread_mutex_unlock(&ctx.mtx);
  return ctx.shared_value;
}

/*
Task 3. Implement a thread-safe stack.
Note that in the mutexes.h file, you can add fields to the stack

Useful links:
std::vector: https://en.cppreference.com/w/cpp/container/vector
pthread mutex: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-110.html
*/

/*
push saves value to the top of the stack
*/
void stack::push(int value) {
  pthread_mutex_lock(&mtx);
  arg.push_back(value);
  pthread_mutex_unlock(&mtx);
}

/*
pop removes a value from the top of the stack and returns it from the function
*/
int stack::pop() {
  pthread_mutex_lock(&mtx);
  int res = arg.back();
  arg.pop_back();
  pthread_mutex_unlock(&mtx);
  return res;
}

/*
Task 4. Implement a multithreaded merge sort that will outperform a
single-threaded implementation Split the array into 4 parts, sort each part in a
separate thread. And then combine the results in merge_sort

Useful links:
merge sort: https://en.wikipedia.org/wiki/Merge_sort
pthread create: https://man7.org/linux/man-pages/man3/pthread_create.3.html
*/

void* m_sort(void* arg) {
  std::vector<int>& values = *((std::vector<int>*)arg);
  sort(values.begin(), values.end());
  return nullptr;
}

void merge_sort(std::vector<int>& values) {
  std::vector<std::vector<int>> split;
  int vector_size = (int)values.size();
  vector_size = std::max(vector_size >> 2, 1);

  for (int i = 0; i < (int)values.size(); i += vector_size) {
    int j = std::min(i + vector_size, (int)values.size());
    if (i < j) {
      split.emplace_back(std::vector<int>());
      for (int iter = i; iter < j; ++iter) {
        split.back().emplace_back(values[iter]);
      }
    }
  }

  int m = (int)split.size();
  std::vector<pthread_t> t(m);
  for (int i = 0; i < m; ++i) {
    pthread_create(&t[i], nullptr, &m_sort, &split[i]);
  }

  for (int i = 0; i < m; ++i) {
    pthread_join(t[i], nullptr);
  }

  while ((int)split.size() > 1) {
    std::vector<int> a = split.back();
    split.pop_back();
    std::vector<int> b = split.back();
    split.pop_back();
    std::vector<int> vec((int)a.size() + (int)b.size());
    merge(a.begin(), a.end(), b.begin(), b.end(), vec.begin());
    split.emplace_back(vec);
  }

  if ((int)split.size() == 1) {
    values = split[0];
  }
}
