#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "../producer_consumer.h"
#include "Test_conf.h"

TEST_CASE("just_example") { CHECK(4 == 4); }

TEST_CASE("check_sum") {
  // start N threads and wait until they're done
  int number;
  bool completed = false;
  bool consumer_started = false;
  bool new_number = false;
  int amount = 0;
  pthread_mutex_t number_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
  pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

  pthread_t producer;
  producer_args p_args;
  p_args.number_ptr = &number;
  p_args.n = 1;
  p_args.completed = &completed;
  p_args.consumer_started = &consumer_started;
  p_args.new_number = &new_number;
  p_args.number_mutex = &number_mutex;
  p_args.producer_cond = &producer_cond;
  p_args.consumer_cond = &consumer_cond;
  pthread_create(&producer, NULL, producer_check, (void*)&p_args);

  pthread_t consumers;
  consumer_args c_args;
  consumer_args consumers_result;

  c_args.debug = false;
  c_args.completed = &completed;
  c_args.consumer_started = &consumer_started;
  c_args.new_number = &new_number;
  c_args.ms = 0;
  c_args.psum = 0;
  c_args.tid = 4 + 1;
  c_args.number_ptr = &number;
  c_args.number_mutex = &number_mutex;
  c_args.producer_cond = &producer_cond;
  c_args.consumer_cond = &consumer_cond;
  pthread_create(&consumers, NULL, consumer_routine, (void*)&c_args);

  // return aggregated sum of values

  pthread_join(consumers, (void**)&consumers_result);
  amount += c_args.psum;
  CHECK(exp_res == amount);
  pthread_mutex_destroy(&number_mutex);
  pthread_cond_destroy(&producer_cond);
  pthread_cond_destroy(&consumer_cond);
}