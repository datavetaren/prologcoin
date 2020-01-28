#include "term.hpp"
#include "garbage_collector.hpp"

namespace prologcoin { namespace common {

garbage_collector::live_block_t
garbage_collector::get_live_block(live_t &live, size_t index) {
  return live[index];
  }

void garbage_collector::set_live_block(live_t &live, size_t index, live_block_t block) {
  //  if (live.size() < (index+1)) {
  //    live.resize(index+1);
  //  }
  live[index] = block;
}

garbage_collector::live_block_t
garbage_collector::mark_live_block(live_t &live, size_t block_index) {
  auto block_live = get_live_block(live, block_index);
  if(block_live == nullptr) {
    std::cout << "block_live: " << block_index << "\n";
    //    std::cout << "creating new bitvector\n";
    block_live = new std::bitset<heap_block::MAX_SIZE>();
    set_live_block(live, block_index, block_live);
  }
  return block_live;
}

bool garbage_collector::mark_live_word(live_t &live, size_t index) {
  auto block_index = index / heap_block::MAX_SIZE;
  auto &block_vector = (*mark_live_block(live, block_index));
  auto bit_index = index - (block_index*heap_block::MAX_SIZE);
  if(!block_vector[bit_index]) {
    block_vector.set(bit_index, 1);
    return true;
  } else {
    return false;
  }
}

void garbage_collector::mark(std::vector<ptr_cell *> &roots, live_t &live) {
  std::vector<size_t> worklist;
  //  std::cout << "Pushing roots\n----------\n";
  for(auto &root : roots) {
    //    std::cout << "Pushing index: " << root->index() << "\n";
    worklist.push_back(root->index());
  }
  //  std::cout << "Pushing roots - done\n----------\n";
  while(worklist.size() > 0) {
    auto current = worklist.back();
    worklist.pop_back();
    //    std::cout << "Current index: " << current << "\n";
    if (mark_live_word(live, current)) {
      cell c = heap_[current];
      // Mark extra blocks beloning to DAT, if it spans over many blocks
      if (c.tag() == tag_t::DAT) {
	auto &dc = reinterpret_cast<const dat_cell&>(c);
	size_t nc = dc.num_cells();
	size_t dat_index = current + heap_block::MAX_SIZE;
	size_t block_index = dat_index / heap_block::MAX_SIZE;
	while (dat_index < (current + nc)) {
	  mark_live_block(live, block_index);
	  block_index = block_index + 1;
	  dat_index = dat_index + heap_block::MAX_SIZE;
	}
	mark_live_block(live, (current + nc)/heap_block::MAX_SIZE);
      }
      push_children(current, worklist);
    }
  }
}

void garbage_collector::push_children(size_t index, std::vector<size_t> &worklist) {
  //  std::cout << "Pushing children\n----------\n";
  cell c = heap_[index];
  switch (c.tag()) {
  case tag_t::INT:
  case tag_t::DAT:
    break;
  case tag_t::CON: {
    auto &con = reinterpret_cast<const con_cell &>(c);
    auto arity = con.arity();
    for(int i = 1; i <= arity; i++) {
      //      std::cout << "Pushing index: " << index + i << "\n";
      worklist.push_back(index+i);
    }
    break;
  }
  case tag_t::RFW:
  case tag_t::BIG:
  case tag_t::REF:
  case tag_t::STR: {
    auto &pc = reinterpret_cast<const ptr_cell&>(c);
    //    std::cout << "Pushing index: " << pc.index() << "\n";
    worklist.push_back(pc.index());
  }
  }
  //  std::cout << "Pushing children - done\n----------\n";
}

size_t garbage_collector::calculate_deltas(std::unordered_map<size_t, size_t> &block_index_to_delta,
                                           live_t &live) {
  size_t current_delta = 0;
  size_t size = live.size();
  for(size_t i = 0; i < size; i++) {
    if(get_live_block(live,i) == nullptr) {
      current_delta += heap_block::MAX_SIZE;
    } else {
      std::cout << "Delta Block[" << i << "] = " << current_delta << "\n";
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
  auto &c = block.get(i);
  switch(c.tag()) {
  case tag_t::RFW:
  case tag_t::BIG:
  case tag_t::REF:
  case tag_t::STR: {
    auto &pcell = reinterpret_cast<ptr_cell&>(c);
    size_t old_index = pcell.index();
    size_t new_index = calculate_new_index(block_index_to_delta, old_index);
    if (old_index != new_index) {
      std::cout << "Rewriting cell: " << old_index << " -> " << new_index << "\n";
      pcell.set_index(new_index);
      block.set(i, pcell);
    }
    assert(reinterpret_cast<ptr_cell&>(block.get(i)).index() == new_index);
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
    if(get_live_block(live, i) != nullptr) {
      auto block = heap_.find_block_from_index(i);
      auto &live_block = *get_live_block(live, i);
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
    if (old_index != new_index) {
      std::cout << "Rewriting ROOT: " << old_index << " -> " << new_index << "\n";
      root->set_index(new_index);
    }
  }
}

void garbage_collector::cleanup_blocks(live_t &live) {
  size_t size = live.size();
  size_t new_index = 0;
  for(size_t i = 0; i < size; i++) {
    auto block = heap_.find_block_from_index(i);
    if(get_live_block(live, i) != nullptr) {
      block->set_index(new_index);
      heap_.blocks_[new_index] = block;
      heap_.head_block_ = block;
      //      if(i == 0) {
      //        heap_.head_block_ = block;
      //      }
      new_index++;
      std::cout << "New index: " << new_index << "\n";
    } else {
      std::cout << "Deleting block: " << i << "\n";
      delete block;
    }
  }
  // Must set heap top?

  auto new_heap_size = heap_.size_ - ((size - new_index) * heap_block::MAX_SIZE);
  std::cout << "Heap size: " << heap_.size_ << " -> " << new_heap_size << "\n";
  heap_.size_ = new_heap_size;
  heap_.blocks_.resize(new_index);
  std::cout << "New heap num blocks: " << heap_.num_blocks() << "\n";
}

void garbage_collector::free_live_sets(live_t &live) {
  for (auto live_block : live) {
    delete live_block;
  }
}

size_t garbage_collector::collect(live_t &live, std::vector<ptr_cell *> &roots) {
  std::unordered_map<size_t, size_t> block_index_to_delta;
  size_t result = calculate_deltas(block_index_to_delta, live);
  rewrite_blocks(block_index_to_delta, live);
  rewrite_roots(block_index_to_delta, roots);
  cleanup_blocks(live);
  free_live_sets(live);
  return result/heap_block::MAX_SIZE;
}

void garbage_collector::dump_marked(live_t &live) {
}

size_t garbage_collector::do_collection(std::vector<ptr_cell *> &roots)
{
  live_t live;
  live.resize(heap_.num_blocks());
  std::cout << "num_blocks: " << heap_.num_blocks() << "\n";
  for(int i = 0; i < live.size(); i++) {
    live[i] = nullptr;
  }
  mark(roots, live);
  auto collected = collect(live, roots);

  std::cout << "Num collected blocks: " << collected << "\n";
  return collected;
}

}}
