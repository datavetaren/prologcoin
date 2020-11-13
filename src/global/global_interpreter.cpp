#include "global_interpreter.hpp"
#include "builtins.hpp"
#include "../common/checked_cast.hpp"
#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "global.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace global {

global_interpreter::global_interpreter(global &g)
    : global_(g),
      current_block_index_(static_cast<size_t>(-2)),
      current_block_(nullptr),
      block_flusher_(),
      block_cache_(global::BLOCK_CACHE_SIZE / heap_block::MAX_SIZE / sizeof(cell), block_flusher_),
      new_predicates_(0),
      new_frozen_closures_(0),
      old_heap_size_(0),
      next_atom_id_(0),
      start_next_atom_id_(0),
      next_predicate_id_(0),
      start_next_predicate_id_(0)
{
    set_auto_wam(true);
}

void global_interpreter::init()
{
    heap_setup_get_block_function( call_get_heap_block, this );

    init_from_heap_db();
    init_from_symbols_db();
    init_from_program_db();

    heap_setup_new_atom_function( call_new_atom, this );
    heap_setup_modified_block_function( call_modified_heap_block, this );

    setup_standard_lib();
    set_retain_state_between_queries(true);

    // TODO: Remove this two lines
    load_builtins_file_io();
    set_debug_enabled();

    setup_consensus_lib(*this);

    setup_builtins();

    // Clear updated/old predicates (side effect for initializing interpreter)
    updated_predicates_.clear();
    old_predicates_.clear();
    new_predicates_ = 0;
    old_heap_size_ = heap_size();
}

void global_interpreter::total_reset()
{
    block_cache_.clear();

    naming_ = false;
    name_to_term_.clear();
    current_block_index_ = static_cast<size_t>(-2);
    current_block_ = nullptr;
    new_atoms_.clear();
    modified_blocks_.clear();
    updated_predicates_.clear();
    old_predicates_.clear();
    modified_closures_.clear();

    global::erase_db(get_global().data_dir());
    interpreter::total_reset();
    init();
}

void global_interpreter::load_predicate(const interp::qname &qn) {
    std::vector<term> clauses;
    if (get_global().db_get_predicate(qn, clauses)) {
	auto *pred = internal_get_predicate(qn);
	for (auto clause : clauses) {
	    pred->add_clause(*this, clause);
	}
    } else {
	// The predicate was not found, but maybe there's an inherited
	// system predicate available? Of course, the global interpreter
	// is not allowed to shadow system: predicates, but I'd like to keep
	// that rule separate from the general infrastructure.
	// Also note that this rule is quite different from standard Prolog
	// which only import system predicates at "use_module(...)", but
	// the global interpreter may have millions of predicates and we
	// don't want to load them all into memory (only by demand) so that's
	// why the rule is slightly different for the global interpreter.
	// Think "use_module(system)" but applied lazily.
	static const con_cell SYSTEM("system",0);
	interp::qname imported_qn(SYSTEM, qn.second);
	if (get_global().db_get_predicate(imported_qn, clauses)) {
	    auto *pred = internal_get_predicate(qn);
	    for (auto clause : clauses) {
		pred->add_clause(*this, clause);
	    }
	    import_predicate(qn); // Here's the lazy rule
	}
    }
}

size_t global_interpreter::unique_predicate_id(const common::con_cell /*module*/) {
    size_t predicate_id = next_predicate_id_;
    next_predicate_id_++;
    return predicate_id;
}

void global_interpreter::setup_consensus_lib(interpreter &interp) {
  ec::builtins::load_consensus(interp);
  coin::builtins::load_consensus(interp);
  builtins::load(interp); // Overrides reward_2 from coin
    
  std::string lib = R"PROG(

%
% Transaction predicates
%

%
% tx/5
%

tx(CoinIn, Hash, Script, Args, CoinOut) :-
    functor(CoinIn, Functor, Arity),
    arg(1, CoinIn, V),
    ground(V),
    arg(2, CoinIn, X),
    var(X),
    cmove(CoinIn, CoinX),
    freeze(Hash,
           (call(Script, Hash, Args),
            CoinOut = CoinX)).

tx1(Hash,args(Signature,PubKey,PubKeyAddr)) :-
    ec:address(PubKey,PubKeyAddr),
    ec:validate(PubKey,Hash,Signature).

reward(PubKeyAddr) :-
    reward(_, Coin),
    tx(Coin, _, tx1, args(_,_,PubKeyAddr), _).

)PROG";

    auto &tx_5 = interp.get_predicate(con_cell("user",0), con_cell("tx",5));
    if (tx_5.empty()) {
        // Nope, so load it
        interp.load_program(lib);
	auto &tx_5_verify = interp.get_predicate(con_cell("user",0), con_cell("tx",5));
	assert(!tx_5_verify.empty());

	// The non-existance of tx_5 means we've created a genesis block
	// So we'll capture the state so it cannot be removed/discarded.
        reinterpret_cast<global_interpreter &>(interp).get_global().advance();
    }
    interp.compile();
}
  
bool global_interpreter::execute_goal(term t) {
    bool r = execute(t);
    // If no choicepoints, then clean up the trail
    if (!has_more()) {
	set_register_hb(static_cast<size_t>(0));
	tidy_trail();
    }
    return r;
}

bool global_interpreter::execute_goal(buffer_t &serialized, bool silent)
{
    term_serializer ser(*this);

    try {
        term goal = ser.read(serialized);
	
	if (naming_ && !silent) {
	    std::unordered_set<std::string> seen;
	    // Scan all vars in goal, and set initial bindings
	    std::for_each( begin(goal),
		   end(goal),
		   [&](term t) {
		     if (t.tag().is_ref()) {
		           ref_cell r = reinterpret_cast<ref_cell &>(t).unwatch();
			   const std::string name = to_string(r);
			   if (!seen.count(name)) {
			       seen.insert(name);
			       if (name_to_term_.count(name)) {
				   unify(t, name_to_term_[name]);
			       } else {
				   name_to_term_[name] = t;
			       }
			   }
		       }
		   } );
	}

	if (!execute_goal(goal)) {
	    reset();
	    return false;
	}
	if (!silent) {
	    serialized.clear();
	    ser.write(serialized, goal);
	}
	return true;
    } catch (serializer_exception &ex) {
	reset();
        throw ex;
    } catch (interpreter_exception &ex) {
	reset();
        throw ex;
    }
}

void global_interpreter::execute_cut() {
    set_b0(nullptr); // Set cut point to top level
    interpreter_base::cut();
    interpreter_base::clear_trail();
}

void global_interpreter::discard_changes() {
    // Discard program changes
    for (auto &qn : updated_predicates_) {
	std::cout << "clear predicate " << to_string(qn) << std::endl;
	clear_predicate(qn);
	remove_compiled(qn);
    }
    updated_predicates_.clear();

    // Restore old predictaes (if any)
    for (auto &p : old_predicates_) {
	if (!p.empty()) {
	    std::cout << "restore predicate " << to_string(p.qualified_name()) << std::endl;
	}
	restore_predicate(p);
    }
    old_predicates_.clear();

    // Tell that no program changes have occurred so that
    // heap can be properly trimmed.
    heap_limit(old_heap_size_);

    // Unwind everything on heap (unbind variables, etc.)
    reset();

    // Discard heap changes
    for (auto &m : modified_blocks_) {
	auto block_index = m.first;
	auto block = m.second;
	block->clear_changed();
	block_cache_.erase(block_index);
    }
    modified_blocks_.clear();

    // Discard new symbols
    for (auto &sym : new_atoms_) {
	clear_atom_index(sym.second, sym.first);
    }

    // Reset symbol counters
    next_atom_id_ = start_next_atom_id_;
    next_predicate_id_ = start_next_predicate_id_;
    new_predicates_ = 0;
    trim_heap_safe(old_heap_size_);
    get_stacks().reset(); // Clear trail and stacks
}
    
bool global_builtins::operator_clause_2(interpreter_base &interp0, size_t arity, term args[] )
{
    auto &interp = to_global(interp0);

    term head = args[0];
    term body = args[1];

    if (head.tag() != tag_t::STR || interp.functor(head) != con_cell("p",1)) {
        throw interpreter_exception_wrong_arg_type(":-/2: Head of clause must be 'p(Hash)'; was " + interp.to_string(head));
    }

    // Setup new environment and where to continue
    interp.allocate_environment<ENV_NAIVE>();
	
    interp.set_p(code_point(body));
    interp.set_cp(code_point(interpreter_base::EMPTY_LIST));

    return true;
}

void global_interpreter::setup_builtins()
{
    load_builtin(con_cell(":-",2), &global_builtins::operator_clause_2);
}

size_t global_interpreter::new_atom(const std::string &atom_name)
{
    // Let's see if this symbol is already known
    auto index = get_global().db_get_symbol_index(atom_name);
    if (index != 0) {
	return index;
    }

    size_t atom_id = next_atom_id_;
    next_atom_id_++;
    set_atom_index(atom_name, atom_id);
    new_atoms_.push_back(std::make_pair(atom_id, atom_name));
    return atom_id;
}

heap_block * global_interpreter::db_get_heap_block(size_t block_index) {
    common::heap_block *block = get_global().db_get_heap_block(block_index);
    block_cache_.insert(block_index, block);
    current_block_ = block;
    return block;
}

term global_interpreter::get_frozen_closure(size_t addr)
{
    auto it = modified_closures_.find(addr);
    if (it != modified_closures_.end()) {
	return it->second;
    }
    return get_global().db_get_closure(addr);
}

void global_interpreter::clear_frozen_closure(size_t addr)
{
    bool is_frozen = get_frozen_closure(addr) != EMPTY_LIST;
    if (!is_frozen) {
	return;
    }
    internal_clear_frozen_closure(addr);

    // Check if frozen closure is in modified list, then remove it
    auto it = modified_closures_.find(addr);
    if (it != modified_closures_.end()) {
	new_frozen_closures_--;
	modified_closures_.erase(addr);
	return;
    }
    // We need to remove it from the database, so we annotate for it
    modified_closures_[addr] = term();
    new_frozen_closures_--;
}

void global_interpreter::set_frozen_closure(size_t addr, term closure)
{
    internal_set_frozen_closure(addr, closure);
    modified_closures_[addr] = closure;
    new_frozen_closures_++;
}

void global_interpreter::get_frozen_closures(size_t from_addr,
					     size_t to_addr,
					     size_t max_closures,
			     std::vector<std::pair<size_t,term> > &closures)
{
    auto root = get_global().get_blockchain().closures_root();
    size_t k = max_closures;

    bool reversed = heap_size() == from_addr && to_addr <= from_addr;

    const auto none = term();

    auto &closures_db = get_global().get_blockchain().closures_db();
    
    auto it1 = reversed ? closures_db.end(root) : 
	                  closures_db.begin(root, from_addr);
    if (reversed) --it1;
    size_t prev_last_addr = from_addr;
    size_t last_addr = from_addr;
    while (!it1.at_end() && k > 0) {
	auto &leaf = *it1;
	if (reversed) {
	    if (leaf.key() < to_addr) {
		break;
	    }
	} else {
	    if (leaf.key() >= to_addr) {
		break;
	    }
	}
	prev_last_addr = last_addr;
	last_addr = leaf.key();

	// Process modified closures in between
	if (reversed) {
	    auto it2 = modified_closures_.lower_bound(last_addr+1);
	    while (it2 != modified_closures_.end()) {
		if (it2->first >= prev_last_addr) {
		    break;
		}
		if (it2->second != none) {
		    closures.push_back(*it2);
		}
		++it2;
	    }
	} else {
	    auto it2 = modified_closures_.lower_bound(prev_last_addr+1);
	    while (it2 != modified_closures_.end()) {
		if (it2->first >= last_addr) {
		    break;
		}
		if (it2->second != none) {
		    closures.push_back(*it2);
		}
		++it2;
	    }
	}

	auto mod = modified_closures_.find(leaf.key());
	if (mod != modified_closures_.end()) {
	    ++it1;
	    if (mod->second == none) {
		continue;
	    }
	    k--;
	    closures.push_back(*mod);
	    continue;
	}

	assert(it1->custom_data_size() == sizeof(uint64_t));
	cell cl(db::read_uint64(it1->custom_data()));
	closures.push_back(std::make_pair(leaf.key(), cl));
	k--;
	if (reversed) --it1; else ++it1;
    }

    if (!reversed) {
	// At this point we check modified closures from last_addr
	auto mod = modified_closures_.lower_bound(last_addr);
	while (mod != modified_closures_.end() && k > 0) {
	    if (mod->first >= to_addr) {
		break;
	    }
	    if (mod->second != none) {
		closures.push_back(*mod);
		k--;
	    }
	    ++mod;
	}
    } else if (last_addr > 0) {
	auto mod0 = modified_closures_.lower_bound(last_addr-1);
	std::map<size_t, term>::reverse_iterator mod(mod0);
	while (mod != modified_closures_.rend() && k > 0) {
	    if (mod->first < to_addr) {
		break;
	    }
	    if (mod->second != none) {
		closures.push_back(*mod);
		k--;
	    }
	    ++mod;
	}
    }
}

void global_interpreter::commit_symbols()
{
    if (new_atoms_.empty()) {
	 return;
    }
    
    for (auto &a : new_atoms_) {
	size_t symbol_index = a.first;
        const std::string &symbol_name = a.second;
	get_global().db_set_symbol_index(symbol_index, symbol_name);
    }

    new_atoms_.clear();
}

void global_interpreter::commit_closures()
{
    const auto none = term();
    for (auto &cl : modified_closures_) {
	if (cl.second == none) {
	    get_global().db_remove_closure(cl.first);
	} else {
	    get_global().db_set_closure(cl.first, cl.second);
	}
    }
    modified_closures_.clear();
    new_frozen_closures_ = 0;
}

size_t global_interpreter::num_predicates() const {
    return get_global().db_num_predicates() + new_predicates_;
}

size_t global_interpreter::num_symbols() {
    return get_global().db_num_symbols() + new_atoms_.size();
}

size_t global_interpreter::num_frozen_closures() const {
    return get_global().db_num_frozen_closures() + new_frozen_closures_;
}

void global_interpreter::commit_heap()
{
    std::vector<heap_block*> blocks;
    for (auto &e : modified_blocks_) {
        auto *block = e.second;
	blocks.push_back(block);
    }
    std::sort(blocks.begin(), blocks.end(),
	   [](heap_block *a, heap_block *b) { return a->index() < b->index();});
    for (auto *block : blocks) {
	get_global().db_set_heap_block(block->index(), block);
	block->clear_changed();
	block_cache_.insert(block->index(), block);
    }
    modified_blocks_.clear();
    old_heap_size_ = heap_size();
}

void global_interpreter::init_from_heap_db()
{
    auto &hdb = get_global().get_blockchain().heap_db();
    if (hdb.is_empty()) {
	new_heap();
        return;
    }
    auto root = get_global().get_blockchain().heap_root();
    if (root.is_zero() ||
	get_global().get_blockchain().heap_db().num_entries(root) == 0) {
	new_heap();
	return;
    }
		
    auto it = hdb.end(root);
    --it;
    if (it.at_end()) {
	new_heap();
	return;
    }
    assert(!it.at_end());
    auto &leaf = *it;
    auto block_index = leaf.key();
    auto &block = get_heap_block(block_index);
    size_t heap_size = block_index * heap_block::MAX_SIZE + block.size();
    heap_set_size(heap_size);
    set_head_block(&block);
}

void global_interpreter::new_heap() {
    assert(heap_size() == 0);
    auto &block = get_heap_block(common::heap::NEW_BLOCK);
    set_head_block(&block);
} 

void global_interpreter::init_from_symbols_db()
{
    next_atom_id_ = get_global().db_num_symbols();
    start_next_atom_id_ = next_atom_id_;
}
    
void global_interpreter::init_from_program_db()
{
    next_predicate_id_ = get_global().db_num_predicates();
    start_next_predicate_id_ = next_predicate_id_;
}
    
void global_interpreter::commit_program()
{    
    if (updated_predicates_.empty()) {
	 return;
    }

    for (auto &qn : updated_predicates_) {
        auto &pred = get_predicate(qn);
	auto &mclauses = pred.get_clauses();
	std::vector<term> clauses;
	for (auto &mc : mclauses) {
	    clauses.push_back(mc.clause());
	}
	get_global().db_set_predicate(qn, clauses);
    }

    updated_predicates_.clear();
    old_predicates_.clear();
    new_predicates_ = 0;
}

}}
