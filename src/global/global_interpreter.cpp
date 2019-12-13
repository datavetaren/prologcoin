#include "global_interpreter.hpp"
#include "builtins.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace global {

global_interpreter::global_interpreter() {
    builtins::load(*this);
    setup_standard_lib();
    setup_special_lib();
    set_retain_state_between_queries(true);

    // TODO: Remove this
    load_builtins_file_io();
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
		   [&](const term &t) {
		       if (t.tag() == tag_t::REF) {
			   const std::string name = to_string(t);
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
        return false;
    } catch (interpreter_exception &ex) {
        return false;
    }
}

void global_interpreter::execute_cut() {
    set_b0(nullptr); // Set cut point to top level
    interpreter_base::cut();
    interpreter_base::clear_trail();
}
    

}}
