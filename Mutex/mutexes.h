#pragma once

#include <pthread.h>
#include <cstdint>
#include <vector>

// 1. Task
void thread_safe_increment(int& value);

// 2.Task
struct context_t {
  pthread_cond_t cond;
  pthread_mutex_t mtx;
  int shared_value;
  bool is_provided;
};

void provide(context_t& ctx, int value);

int consume(context_t& ctx);

// 3. Task
class stack {
  /*
  Add here any fields that are needed for implementation.
  Recommended fields are pthread_mutex_t and std::vector<int>

  Useful links:
  std::vector: https://en.cppreference.com/w/cpp/container/vector
  pthread mutex: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-110.html
  */

  std::vector<int> arg;

 public:
  pthread_mutex_t mtx;

  void push(int value);

  int pop();
};

// 4. Task
void merge_sort(std::vector<int>& values);
