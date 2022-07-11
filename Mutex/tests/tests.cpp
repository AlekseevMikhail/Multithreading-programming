#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <mutexes.h>
#include <chrono>
#include <thread>

TEST_CASE("task1_thread_safe_incerement_seq") {
  int a = 5;
  thread_safe_increment(a);
  REQUIRE(a == 6);
  thread_safe_increment(a);
  REQUIRE(a == 7);
  thread_safe_increment(a);
  REQUIRE(a == 8);
}

TEST_CASE("task1_thread_safe_incerement_concurrent") {
  for (size_t attempt = 0; attempt < 10; attempt++) {
    int a = 0;
    std::thread t1{[&a] {
      for (size_t i = 0; i < 1000000; i++) {
        thread_safe_increment(a);
      }
    }};
    std::thread t2{[&a] {
      for (size_t i = 0; i < 1000000; i++) {
        thread_safe_increment(a);
      }
    }};
    std::thread t3{[&a] {
      for (size_t i = 0; i < 1000000; i++) {
        thread_safe_increment(a);
      }
    }};
    t1.join();
    t2.join();
    t3.join();
    REQUIRE(a == 3000000);
  }
}

TEST_CASE("task2_provide_consume") {
  for (size_t attempt = 0; attempt < 10; attempt++) {
    context_t ctx{};
    int result = 0;
    std::thread t1{[&ctx, &result] { result = consume(ctx); }};
    std::thread t2{[&ctx, attempt] { provide(ctx, attempt); }};
    t1.join();
    t2.join();
    REQUIRE(result == attempt);
  }
}

TEST_CASE("task3_stack") {
  for (size_t attempt = 0; attempt < 10; attempt++) {
    stack st;
    int result1 = 0;
    std::thread t1{[&st, &result1] {
      for (size_t i = 0; i <= 10000; i++) {
        st.push(i);
      }
      for (size_t i = 0; i <= 10000; i++) {
        result1 += st.pop();
      }
    }};
    int result2 = 0;
    std::thread t2{[&st, &result2] {
      for (size_t i = 0; i <= 10000; i++) {
        st.push(i);
      }
      for (size_t i = 0; i <= 10000; i++) {
        result2 += st.pop();
      }
    }};
    t1.join();
    t2.join();
    REQUIRE(result1 + result2 == 2 * (1 + 10000) * 5000);
  }
}

TEST_CASE("task4_just_sort") {
  for (size_t attempt = 0; attempt < 10; attempt++) {
    std::vector<int> values;
    for (size_t value = 0; value < attempt; value++) {
      values.push_back(value);
    }
    std::random_shuffle(values.begin(), values.end());
    merge_sort(values);
    for (size_t value = 0; value < attempt; value++) {
      REQUIRE(values[value] == value);
    }
  }
}

TEST_CASE("task4_performance") {
  std::vector<int> values1;
  std::vector<int> values2;
  for (size_t layer = 0; layer < 10; layer++) {
    for (size_t i = 0; i < 100000; i++) {
      values1.push_back(i);
      values2.push_back(i);
    }
  }
  std::random_shuffle(values2.begin(), values2.end());

  std::chrono::steady_clock::time_point serial_begin =
      std::chrono::steady_clock::now();
  for (size_t i = 0; i < 18; i++) {
    std::random_shuffle(values1.begin(), values1.end());
    std::sort(values1.begin(), values1.end());
  }
  std::chrono::steady_clock::time_point serial_end =
      std::chrono::steady_clock::now();

  std::chrono::steady_clock::time_point parallel_begin =
      std::chrono::steady_clock::now();
  merge_sort(values2);
  std::chrono::steady_clock::time_point parallel_end =
      std::chrono::steady_clock::now();
  REQUIRE(values1 == values2);
  REQUIRE((serial_end - serial_begin).count() >
          (parallel_end - parallel_begin).count());
}
