#pragma once
#ifndef _common_garbage_collector_hpp
#define _common_garbage_collector_hpp

#include <bitset>
#include <vector>
#include <map>

namespace prologcoin { namespace common {

class garbage_collector {

public:
  garbage_collector(heap &h) : heap_(h) {};
  // Garbage collects and returns number of collected bytes.
  size_t do_collection(std::vector<ptr_cell *> &roots);

private:
  typedef std::bitset<heap_block::MAX_SIZE> * live_block_t;
  typedef std::vector<live_block_t> live_t;
  heap &heap_;

  live_block_t get_live_block(live_t &live, size_t index);
  void set_live_block(live_t &live, size_t index, live_block_t block);

  live_block_t mark_live_block(live_t &live, size_t block_index);
  bool mark_live_word(live_t &live, size_t index);
  void mark(std::vector<ptr_cell *> &roots, live_t &live);
  void push_children(size_t index, std::vector<size_t> &worklist);

  size_t calculate_deltas(std::unordered_map<size_t, size_t> &block_index_to_delta,
                          live_t &live);
  size_t calculate_new_index(std::unordered_map<size_t, size_t> &block_index_to_delta,
			     size_t index);

  void rewrite_cell(size_t i, heap_block &block,
                    std::unordered_map<size_t, size_t> &block_index_to_delta);
  void rewrite_blocks(std::unordered_map<size_t, size_t> &block_index_to_delta,
                              live_t &live);
  void rewrite_roots(std::unordered_map<size_t, size_t> &block_index_to_delta,
		     std::vector<ptr_cell *> &roots);
  void cleanup_blocks(live_t &live);
  void free_live_sets(live_t &live);
  void dump_marked(live_t &live);
  size_t collect(live_t &live, std::vector<ptr_cell *> &roots);
};

}}
#endif
