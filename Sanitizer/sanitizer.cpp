#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "sanitizer.h"

using namespace std;

extern "C" {
// your c code
// linkage naming

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static int (*orig_pthread_mutex_lock)(pthread_mutex_t *mutex) =
    (int (*)(pthread_mutex_t *mutex))dlsym(RTLD_NEXT, "pthread_mutex_lock");
static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *mutex) =
    (int (*)(pthread_mutex_t *mutex))dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

// your c++ code

int MutexGraph::find_node(pthread_mutex_t *mtx) {
  for (int i = 0; i < int(MutexGraph::graph_nodes.size()); ++i) {
    GraphNode &cur_node = MutexGraph::graph_nodes[i];
    if (cur_node.mutex == mtx) {
      return i;
    }
  }
  return -1;
}

void MutexGraph::reset_nodes() {
  for (auto &cur_node : MutexGraph::graph_nodes) {
    cur_node.color = WHITE;
  }
}

void MutexGraph::create_node(pthread_mutex_t *mtx) {
  int pos = MutexGraph::find_node(mtx);
  if (pos == -1) {
    MutexGraph::graph_nodes.emplace_back(GraphNode(mtx));
  }
}

void MutexGraph::add_edge(pthread_mutex_t *from, pthread_mutex_t *to) {
  int it_from = MutexGraph::find_node(from);
  int it_to = MutexGraph::find_node(to);
  MutexGraph::graph_nodes[it_from].next_nodes_position.emplace_back(it_to);
}

void MutexGraph::lock_graph() {
  orig_pthread_mutex_lock(&(MutexGraph::graph_mutex));
}

void MutexGraph::unlock_graph() {
  orig_pthread_mutex_unlock(&(MutexGraph::graph_mutex));
}

bool MutexGraph::find_cycle(int cur_node_position,
                            vector<pthread_mutex_t *> cur_route) {
  GraphNode &cur_node = MutexGraph::graph_nodes[cur_node_position];
  cur_node.color = GRAY;
  cur_route.emplace_back(cur_node.mutex);
  for (int next_position : cur_node.next_nodes_position) {
    GraphNode &next_node = MutexGraph::graph_nodes[next_position];
    if (next_node.color == GRAY) {
      if (MutexGraph::cycle_nodes.empty()) {
        MutexGraph::cycle_nodes = cur_route;
      }
      return true;
    }
    if (next_node.color == WHITE &&
        MutexGraph::find_cycle(next_position, cur_route)) {
      if (MutexGraph::cycle_nodes.empty()) {
        MutexGraph::cycle_nodes = cur_route;
      }
      return true;
    }
  }
  cur_node.color = BLACK;
  cur_route.pop_back();
  return false;
}

bool MutexGraph::find_cycle() {
  MutexGraph::reset_nodes();
  MutexGraph::cycle_nodes.clear();
  MutexGraph::cycle_flag = false;
  for (int i = 0; i < int(MutexGraph::graph_nodes.size()); ++i) {
    MutexGraph::cycle_flag = MutexGraph::find_cycle(i, {});
    if (MutexGraph::cycle_flag) {
      break;
    }
  }
  return MutexGraph::cycle_flag;
}

void MutexGraph::print_cycle() {
  printf("Cycle found:\n\n");
  for (int i = int(MutexGraph::cycle_nodes.size()) - 1; i >= 0; --i) {
    pthread_mutex_t *mtx = MutexGraph::cycle_nodes[i];
    printf("Mutex addr (%p)", (void *)mtx);
    printf(" ===> ");
  }
  printf("Mutex addr (%p)", (void *)(MutexGraph::cycle_nodes.back()));
  printf("\n\n");
}

static MutexGraph mtx_graph = MutexGraph();
static thread_local vector<pthread_mutex_t *> acquired_mutex;

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  bool cycle = false;
  mtx_graph.lock_graph();

  mtx_graph.create_node(mutex);
  if (!acquired_mutex.empty()) {
    mtx_graph.add_edge(acquired_mutex.back(), mutex);
  }
  if (mtx_graph.find_cycle()) {
    mtx_graph.print_cycle();
    cycle = true;
  }
  acquired_mutex.emplace_back(mutex);

  mtx_graph.unlock_graph();
  if (cycle) {
    exit(EXIT_FAILURE);
  }
  return orig_pthread_mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  mtx_graph.lock_graph();
  auto it = find(acquired_mutex.begin(), acquired_mutex.end(), mutex);
  if (it != acquired_mutex.end()) {
    acquired_mutex.erase(it);
  }
  mtx_graph.unlock_graph();
  return orig_pthread_mutex_unlock(mutex);
}
