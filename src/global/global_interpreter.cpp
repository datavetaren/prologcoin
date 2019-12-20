#include "global_interpreter.hpp"
#include "builtins.hpp"
#include "../ec/builtins.hpp"
#include "../coin/builtins.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global_interpreter::global_interpreter() {
    builtins::load(*this);
    setup_standard_lib();
    set_retain_state_between_queries(true);

    // TODO: Remove this
    load_builtins_file_io();

    setup_consensus_lib(*this);
}

void global_interpreter::setup_consensus_lib(interpreter &interp) {
  ec::builtins::load_consensus(interp);
  coin::builtins::load_consensus(interp);
    
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
    X = [],
    freeze(Hash,
           (call(Script, Hash, Args),
            functor(CoinOut, Functor, Arity),
            arg(1, CoinOut, V))).

tx1(Hash,args(Signature,PubKey,PubKeyAddr)) :-
    ec:address(PubKey,PubKeyAddr),
    ec:validate(PubKey,Hash,Signature).

)PROG";

    interp.load_program(lib);
    interp.compile();
}
  
bool global_interpreter::execute_goal(term t) {
    return execute(t);
}

bool global_interpreter::execute_goal(buffer_t &serialized)
{
    term_serializer ser(*this);
    try {
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
	    return false;
	}
	serialized.clear();
	ser.write(serialized, goal);
	return true;
    } catch (serializer_exception &ex) {
        throw ex;
    } catch (interpreter_exception &ex) {
        throw ex;
    }
}

void global_interpreter::execute_cut() {
    set_b0(nullptr); // Set cut point to top level
    interpreter_base::cut();
    interpreter_base::clear_trail();
}
    

}}
