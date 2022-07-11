#pragma once

#include <map>
#include <memory>
#include <pthread.h>
#include <set>
#include <vector>

using namespace std;
// TODO: write declation here

enum Colors { WHITE, GRAY, BLACK };

struct GraphNode {
public:
  pthread_mutex_t *mutex;
  Colors color;
  vector<int> next_nodes_position;

public:
  GraphNode() : mutex(nullptr) {}
  GraphNode(pthread_mutex_t *init_mutex) : mutex(init_mutex), color(WHITE) {}
};

class MutexGraph {
public:
  pthread_mutex_t graph_mutex;
  vector<GraphNode> graph_nodes;
  vector<pthread_mutex_t *> cycle_nodes;
  bool cycle_flag;

public:
  MutexGraph() {
    graph_mutex = PTHREAD_MUTEX_INITIALIZER;
    cycle_flag = false;
  }
  int find_node(pthread_mutex_t *);
  void reset_nodes();
  void create_node(pthread_mutex_t *);
  void add_edge(pthread_mutex_t *, pthread_mutex_t *);
  void lock_graph();
  void unlock_graph();
  bool find_cycle(int, vector<pthread_mutex_t *>);
  bool find_cycle();
  void print_cycle();
};
