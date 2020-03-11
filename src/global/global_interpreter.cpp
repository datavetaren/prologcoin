#include "global_interpreter.hpp"
#include "builtins.hpp"
#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"
#include "global.hpp"

using namespace prologcoin::common;
using namespace prologcoin::interp;

namespace prologcoin { namespace global {

global_interpreter::global_interpreter(global &g) : global_(g) {
    heap_setup_get_block_function( global::call_get_heap_block, &global_ );
    heap_setup_modified_block_function( global::call_modified_heap_block, &global_ );
    
    setup_standard_lib();
    set_retain_state_between_queries(true);

    // TODO: Remove this
    load_builtins_file_io();

    setup_consensus_lib(*this);

    setup_builtins();
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
    }
    interp.compile();
}
  
bool global_interpreter::execute_goal(term t, bool and_undo) {
    if (and_undo) {
        allocate_choice_point(code_point::fail());
    }  
    bool r = execute(t);
    if (and_undo) unwind_to_top_choice_point();
    return r;
}

bool global_interpreter::execute_goal(buffer_t &serialized, bool and_undo)
{
    term_serializer ser(*this);

    size_t old_height = get_global().current_height();
    
    try {
	if (and_undo) {
	    allocate_choice_point(code_point::fail());
	}
	
        term goal = ser.read(serialized);

	if (naming_) {
	    std::unordered_set<std::string> seen;
	    // Scan all vars in goal, and set initial bindings
	    std::for_each( begin(goal),
		   end(goal),
		   [&](term t) {
		     if (t.tag().is_ref()) {
		           ref_cell r = static_cast<ref_cell &>(t).unwatch();
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

	if (!execute(goal)) {
	    get_global().set_height(old_height);
	    if (and_undo) {
	        reset();
	    }
	    return false;
	}
	serialized.clear();
	ser.write(serialized, goal);
	if (and_undo) {
	    get_global().set_height(old_height);	
	    reset();
	}
	return true;
    } catch (serializer_exception &ex) {
	get_global().set_height(old_height);
        throw ex;
    } catch (interpreter_exception &ex) {
	get_global().set_height(old_height);      
        throw ex;
    }
}

void global_interpreter::execute_cut() {
    set_b0(nullptr); // Set cut point to top level
    interpreter_base::cut();
    interpreter_base::clear_trail();
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
	
    interp.set_p(body);
    interp.set_cp(interpreter_base::EMPTY_LIST);

    return true;
}

void global_interpreter::setup_builtins()
{
    load_builtin(con_cell(":-",2), &global_builtins::operator_clause_2);
}
    
}}
