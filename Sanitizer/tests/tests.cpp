#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <sanitizer.h>

TEST_CASE("just_example") { CHECK(4 == 4); }

#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mtx_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_2 = PTHREAD_MUTEX_INITIALIZER;

void *thread_A(void *arg) {
  (void)arg;
  pthread_mutex_lock(&mtx_1);
  pthread_mutex_lock(&mtx_2);
  pthread_mutex_unlock(&mtx_2);
  pthread_mutex_unlock(&mtx_1);

  return nullptr;
}

void *thread_B(void *arg) {
  (void)arg;
  pthread_mutex_lock(&mtx_1);
  pthread_mutex_lock(&mtx_2);
  pthread_mutex_unlock(&mtx_2);
  pthread_mutex_unlock(&mtx_1);

  return nullptr;
}

TEST_CASE("no deadlock") {
  pthread_t A, B;
  pthread_create(&A, nullptr, &thread_A, nullptr);
  pthread_create(&B, nullptr, &thread_B, nullptr);
  int ret_A = pthread_join(A, nullptr);
  int ret_B = pthread_join(B, nullptr);
  CHECK(ret_A == 0);
  CHECK(ret_B == 0);
}
