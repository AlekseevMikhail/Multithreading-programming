//
// Created by misha on 18.05.2022.
//

#ifndef INC_2_TEST_CONF_H
#define INC_2_TEST_CONF_H

#include <pthread.h>
#include <vector>
#include "../producer_consumer.h"
std::vector<int> datatest{1, 2, 3, 4, 5, 6, 7};
int exp_res = 28;

void* producer_check(void* arg) {
  producer_args* args = (producer_args*)arg;

  for (size_t i = 0; i != datatest.size(); i++) {
    pthread_mutex_lock(args->number_mutex);
    *args->number_ptr = datatest[i];
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

#endif  // INC_2_TEST_CONF_H