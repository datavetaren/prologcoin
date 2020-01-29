#include "term.hpp"
#include "garbage_collector.hpp"

namespace prologcoin { namespace common {

void garbage_collector::mark(std::vector<ptr_cell *> &roots, live_t &live) {
  std::vector<size_t> worklist;
  for(auto &root : roots) {
    worklist.push_back(root->index());
  }

  while(worklist.size() > 0) {
    auto current = worklist.back();
    worklist.pop_back();
    auto block_index = current / heap_block::MAX_SIZE;

    auto block_live = live[block_index];
    if(block_live == NULL) {
      block_live = new std::bitset<heap_block::MAX_SIZE>();
      live[block_index] = block_live;
    }

    auto bit_index = current - (block_index*heap_block::MAX_SIZE);
    auto block_vector = *block_live;
    if(!block_vector[bit_index]) {
      block_vector.set(bit_index,1);
      push_children(current, worklist);
    }
  }
}

void garbage_collector::push_children(size_t index,
                                      std::vector<size_t> &worklist) {
  cell c = heap_[index];
  switch (c.tag()) {
  case tag_t::INT:
  case tag_t::DAT:
    break;
  case tag_t::CON: {
    auto &con = reinterpret_cast<const con_cell &>(c);
    auto arity = con.arity();
    for(int i = 1; i <= arity; i++) {
      worklist.push_back(index+i);
    }
    break;
  }
  case tag_t::RFW:
  case tag_t::BIG:
  case tag_t::REF:
  case tag_t::STR: {
    auto &pc = reinterpret_cast<const ptr_cell&>(c);
    worklist.push_back(pc.index());
  }
  }
}

size_t garbage_collector::calculate_deltas(std::unordered_map<size_t, size_t> &block_index_to_delta,
                                           live_t &live) {
  size_t current_delta = 0;
  size_t size = live.size();
  for(size_t i = 0; i < size; i++) {
    if(live[i] == NULL) {
      current_delta += heap_block::MAX_SIZE;;
    } else {
      block_index_to_delta[i] = current_delta;
    }
  }
  return current_delta;
}

size_t garbage_collector::calculate_new_index(std::unordered_map<size_t, size_t> &block_index_to_delta,
					      size_t index) {
  size_t block_index = heap_.find_block_index(index);
  size_t new_index = index - block_index_to_delta[block_index];
  return new_index;
}

void garbage_collector::rewrite_cell(size_t i, heap_block &block,
                                     std::unordered_map<size_t, size_t> &block_index_to_delta) {
  cell c = block.get(i);
  switch(c.tag()) {
  case tag_t::RFW:
  case tag_t::BIG:
  case tag_t::REF:
  case tag_t::STR: {
    auto &pcell = reinterpret_cast<ptr_cell&>(c);
    size_t old_index = pcell.index();
    size_t new_index = calculate_new_index(block_index_to_delta, old_index);
    pcell.set_index(new_index);
    block.set(i, pcell);
    break;
  }
  default:
    break;
  }
}

void garbage_collector::rewrite_blocks(std::unordered_map<size_t, size_t> &block_index_to_delta,
                                       live_t &live) {
  size_t size = live.size();
  for(size_t i = 0; i < size; i++) {
    if(live[i] != NULL) {
      auto block = heap_.find_block_from_index(i);
      auto &live_block = *live[i];
      for(size_t j = 0; j < heap_block::MAX_SIZE; j++) {
        if(live_block[j]) {
          rewrite_cell(j, *block, block_index_to_delta);
        }
      }
    }
  }
}

void garbage_collector::rewrite_roots(std::unordered_map<size_t, size_t> &block_index_to_delta,
				      std::vector<ptr_cell *> &roots) {
  for(auto &root : roots) {
    size_t old_index = root->index();
    size_t new_index = calculate_new_index(block_index_to_delta, old_index);
    root->set_index(new_index);
  }
}

void garbage_collector::cleanup_blocks(live_t &live) {
  size_t size = live.size();
  size_t new_index = 0;
  for(size_t i = 0; i < size; i++) {
    auto block = heap_.find_block_from_index(i);
    if(live[i] != NULL) {
      block->set_index(new_index);
      heap_.blocks_[new_index] = block;
      if(i == 0) {
        heap_.head_block_ = block;
      }
      new_index++;
    } else {
      delete block;
    }
  }
  heap_.blocks_.resize(new_index);
}

size_t garbage_collector::collect(live_t &live) {
  std::unordered_map<size_t, size_t> block_index_to_delta;
  size_t result = calculate_deltas(block_index_to_delta, live);
  rewrite_blocks(block_index_to_delta, live);
  cleanup_blocks(live);
  return result;
}

size_t garbage_collector::do_collection(std::vector<ptr_cell *> &roots)
{
  live_t live;
  mark(roots, live);
  return collect(live);
}

}}
